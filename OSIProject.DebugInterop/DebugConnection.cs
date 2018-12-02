using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Net.Sockets;
using System.Threading;

namespace OSIProject.DebugInterop
{
    public class DebugConnection
    {
        public string Host { get; }
        public int Port { get; }
        public event EventHandler ServerDisconnect;
        public event EventHandler ServerChangeState;
        public event EventHandler ServerExecutionState;
        public event EventHandler ServerStackState;
        public event EventHandler<string> ServerDebugOutput;
        public event EventHandler<string> ServerException;

        private object StateSyncObject = new object();
        private bool _isConnected;
        public bool IsConnected
        {
            get
            {
                lock (StateSyncObject)
                {
                    return _isConnected;
                }
            }
            private set
            {
                lock (StateSyncObject)
                {
                    _isConnected = value;
                }
            }
        }
        private TcpClient _clientConnection;
        private TcpClient ClientConnection
        {
            get
            {
                lock (StateSyncObject)
                {
                    return _clientConnection;
                }
            }
            set
            {
                lock (StateSyncObject)
                {
                    _clientConnection = value;
                }
            }
        }
        private System.IO.BinaryWriter Writer;

        private Thread ListenThread;
        private AutoResetEvent ConnectedEvent = new AutoResetEvent(false);
        private AutoResetEvent ShutdownEvent = new AutoResetEvent(false);
        private SynchronizationContext EventContext { get; }

        public DebugConnection(SynchronizationContext eventContext, string host, int port)
        {
            this.IsConnected = false;
            this.EventContext = eventContext;
            this.Host = host;
            this.Port = port;
        }

        public async Task Connect()
        {
            if (this.IsConnected)
            {
                throw new InvalidOperationException("Already connected.");
            }

            this.ListenThread = new Thread(new ThreadStart(ListenThreadEntrypoint));
            this.ListenThread.Start();
            await this.ConnectedEvent.AsTask();
        }

        public async Task Disconnect()
        {
            if (!IsConnected)
                return;

            this.ClientConnection.Close();
            await this.ShutdownEvent.AsTask();
        }

        private void HandlePacket(PacketHeader header, System.IO.BinaryReader payloadReader)
        {
            System.Diagnostics.Debug.WriteLine("Handling packet of type " + header.Type.ToString());

            if (header.Type == PayloadType.ServerConnected)
            {
                IsConnected = true;
                ConnectedEvent.Set();
            }
            else if (header.Type == PayloadType.ServerDisconnect)
            {
                if (!IsConnected)
                    throw new Exception("Connection attempt was rejected.");
                else
                    this.ClientConnection.Close();
            }
            else if (header.Type == PayloadType.ServerDebugOutput)
            {
                this.ServerDebugOutput?.Invoke(this, new ServerDebugOutputPayload(payloadReader).Output);
            }
            else if (header.Type == PayloadType.ServerException)
            {
                ServerExceptionPayload payload = new ServerExceptionPayload(payloadReader);
                this.ServerException?.Invoke(this, payload.Output);
            }
            else
            {
                payloadReader.ReadBytes(header.PayloadLength);
                System.Diagnostics.Debug.WriteLine("Recieved unimplemented packet of type " + header.Type.ToString());
            }

        }

        public void SendPacket(PacketHeader header, Payload payload)
        {
            System.Diagnostics.Debug.WriteLine("Writing packet of type " + header.Type.ToString());

            lock (StateSyncObject)
            {
                header.Write(Writer);
                if (payload != null)
                    payload.Write(Writer);
                Writer.Flush();
            }
        }

        private void ListenThreadEntrypoint()
        {
            System.IO.MemoryStream ms = new System.IO.MemoryStream();
            System.IO.BinaryReader reader = new System.IO.BinaryReader(ms);
            try
            {
                this.ClientConnection = new TcpClient(this.Host, this.Port);
                this.ClientConnection.NoDelay = true;

                lock (StateSyncObject)
                    Writer = new System.IO.BinaryWriter(this.ClientConnection.GetStream());

                //Thread.Sleep(3000);

                SendPacket(new PacketHeader(0, PayloadType.ClientConnection), new ClientConnectionPayload(ClientConnectionPayload.CurrentVersionMajor, ClientConnectionPayload.CurrentVersionMinor));
                int read = 1;
                while (read > 0)
                {
                    ms.SetLength(4);
                    ms.Position = 0;
                    while (ms.Position < 4)
                    {
                        read = this.ClientConnection.GetStream().Read(ms.GetBuffer(), (int)ms.Position, (int)ms.Length - (int)ms.Position);
                        ms.Position += read;
                        if (read <= 0)
                            break;
                    }
                    if (read <= 0)
                        break;

                    //ms.SetLength(read);
                    ms.Position = 0;
                    PacketHeader header = new PacketHeader(reader);
                    ms.Position = 0;

                    if (header.PayloadLength > 0)
                    {
                        ms.SetLength(header.PayloadLength);
                        while (ms.Position < header.PayloadLength)
                        {
                            read = this.ClientConnection.GetStream().Read(ms.GetBuffer(), (int)ms.Position, (int)ms.Length - (int)ms.Position);
                            ms.Position += read;
                            if (read <= 0)
                                break;
                        }
                        ms.Position = 0;
                        if (read <= 0)
                            break;
                    }

                    if (header.Type == PayloadType.ServerDisconnect)
                    {
                        break;
                    }
                    else
                    {
                        HandlePacket(header, reader);
                    }
                    ms.Position = 0;
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine("Exception in ListenThreadEntrypoint: " + ex.ToString());
            }
            if (!this.IsConnected)
                this.ConnectedEvent.Set();
            this.ClientConnection?.Close();
            this.ClientConnection?.Dispose();
            System.Diagnostics.Debug.WriteLine("Connection closed.");
            this.IsConnected = false;
            //this.ServerDisconnect?.Invo
            //EventContext.Send((state) => { this.ServerDisconnect?.Invoke(null, null); }, null);
            this.ServerDisconnect?.Invoke(this, new EventArgs());
            this.ShutdownEvent.Set();
            ms.Dispose();
            reader.Dispose();
        }
    }

    // From https://stackoverflow.com/a/18766131
    internal static class WaitHandleExtensions
    {
        public static Task AsTask(this WaitHandle handle)
        {
            return AsTask(handle, Timeout.InfiniteTimeSpan);
        }

        public static Task AsTask(this WaitHandle handle, TimeSpan timeout)
        {
            var tcs = new TaskCompletionSource<object>();
            var registration = ThreadPool.RegisterWaitForSingleObject(handle, (state, timedOut) =>
            {
                var localTcs = (TaskCompletionSource<object>)state;
                if (timedOut)
                    localTcs.TrySetCanceled();
                else
                    localTcs.TrySetResult(null);
            }, tcs, timeout, executeOnlyOnce: true);
            tcs.Task.ContinueWith((_, state) => ((RegisteredWaitHandle)state).Unregister(null), registration, TaskScheduler.Default);
            return tcs.Task;
        }
    }
}
