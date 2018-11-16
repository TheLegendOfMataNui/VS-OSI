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
        public event EventHandler ServerDebugOutput;
        public event EventHandler ServerException;

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

        public async void Connect()
        {
            if (this.IsConnected)
            {
                throw new InvalidOperationException("Already connected.");
            }

            this.ListenThread = new Thread(new ThreadStart(ListenThreadEntrypoint));
            this.ListenThread.Start();
            await this.ConnectedEvent.AsTask();
        }

        public async void Disconnect()
        {
            if (!IsConnected)
                return;

            this.ClientConnection.Close();
            await this.ShutdownEvent.AsTask();
        }

        private void HandlePacket(PayloadType type, System.IO.BinaryReader payloadReader)
        {
            System.Diagnostics.Debug.WriteLine("Handling packet of type " + type.ToString());
        }

        private void ListenThreadEntrypoint()
        {
            try
            {
                this.ClientConnection = new TcpClient(this.Host, this.Port);

                System.IO.MemoryStream ms = new System.IO.MemoryStream();
                System.IO.BinaryReader reader = new System.IO.BinaryReader(ms);
                int read = -1;
                while (read > 0)
                {
                    ms.Capacity = 4;
                    read = this.ClientConnection.GetStream().Read(ms.GetBuffer(), 0, 4);
                    if (read == 0)
                        break;
                    ms.Position = 0;
                    PacketHeader header = new PacketHeader(reader);
                    ms.Position = 0;

                    if (header.PayloadLength > 0)
                    {
                        ms.Capacity = header.PayloadLength;
                        read = this.ClientConnection.GetStream().Read(ms.GetBuffer(), 0, header.PayloadLength);
                        ms.Position = 0;
                        if (read == 0)
                            break;
                    }

                    if (header.Type == PayloadType.ServerDisconnect)
                    {
                        break;
                    }
                    else
                    {
                        HandlePacket(header.Type, reader);
                    }
                    ms.Position = 0;
                }
            }
            catch (Exception ex)
            {
                System.Diagnostics.Debug.WriteLine("Exception in ListenThreadEntrypoint: " + ex.ToString());
            }
            this.ClientConnection?.Close();
            this.ClientConnection?.Dispose();
            System.Diagnostics.Debug.WriteLine("Connection closed.");
            this.IsConnected = false;
            this.ShutdownEvent.Set();
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
