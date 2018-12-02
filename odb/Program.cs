using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using OSIProject.DebugInterop;

namespace odb
{
    class Program
    {
        private static DebugConnection Connection;

        static void Main(string[] args)
        {
            Run(args).Wait();
        }

        private static int CurrentLine = 1;
        private static int CurrentColumn = 0;

        private static void BetterWrite(string text)
        {
            //Console.CursorLeft = 0;
            int prevTop = Console.CursorTop;
            int prevCol = Console.CursorLeft;
            Console.CursorTop = CurrentLine;
            Console.CursorLeft = CurrentColumn;
            Console.Write(text);
            CurrentLine = Console.CursorTop;
            CurrentColumn = Console.CursorLeft;
            Console.CursorTop = prevTop;
            Console.CursorLeft = prevCol;
            if (CurrentLine >= Console.WindowHeight)
            {
                int delta = Console.WindowHeight - CurrentLine - 1;
                Console.MoveBufferArea(0, 1 - delta, Console.WindowWidth, Console.WindowHeight - 1, 0, 1);
                CurrentLine += delta;
            }
        }

        private static void BetterWriteLine(string text)
        {
            BetterWrite(text + "\n");
        }

        async static Task Run(string[] args)
        {
            string hostname = "localhost";
            int port = 10001;

            Console.Write("Hostname (Blank for '" + hostname + "') > ");
            string hostnameInput = Console.ReadLine().Trim();
            if (hostnameInput.Length > 0)
                hostname = hostnameInput;
            Console.Write("Remote port (Blank for " + port + ") > ");
            string portInput = Console.ReadLine().Trim();
            if (portInput.Length > 0)
            {
                if (!Int32.TryParse(portInput, out port))
                {
                    Console.WriteLine("Invalid port. Using " + port + ".");
                }
            }

            Console.Clear();
            //Console.CursorTop = 1;

            Connection = new DebugConnection(System.Threading.SynchronizationContext.Current, hostname, port);
            BetterWriteLine("Connecting to " + hostname + ":" + port + "...");
            await Connection.Connect();
            if (!Connection.IsConnected)
            {
                BetterWriteLine("Connection failed.");
            }
            else
            {
                BetterWriteLine("Connected!");
                bool exit = false;
                Connection.ServerDisconnect += (_, __) =>
                {
                    BetterWriteLine("Disconnected by server.");
                    exit = true;
                    //throw new Exception(); // Hack to interrupt the ReadLine() method
                };
                Connection.ServerDebugOutput += (_, output) =>
                {
                    BetterWrite(output);
                };
                
                while (!exit)
                {
                    Console.CursorTop = 0;
                    Console.CursorLeft = 0;
                    Console.Write(hostname + ":" + port + " > ");
                    string command = Console.ReadLine();
                    Console.CursorTop = 0;
                    Console.CursorLeft = 0;
                    Console.Write((hostname + ":" + port + " > ").PadRight(Console.BufferWidth));
                    Console.CursorTop = 0;
                    Console.CursorLeft = 0;
                    Console.Write(hostname + ":" + port + " > ");

                    string[] parts = command.Split(' ');
                    if (parts.Length > 0)
                    {
                        if (parts[0] == "disconnect") // disconnect
                        {
                            exit = true;
                            await Connection.Disconnect();
                        }
                        else if (parts[0] == "break") // break
                        {

                        }
                        else if (parts[0] == "step") // step [over], step in, step out
                        {

                        }
                        else if (parts[0] == "resume") // resume
                        {

                        }
                        else if (parts[0] == "stack") // stack [trace], stack locals, stack values
                        {

                        }
                    }
                }
            }
            BetterWriteLine("Press any key to exit.");
            Console.ReadKey();
        }
    }
}
