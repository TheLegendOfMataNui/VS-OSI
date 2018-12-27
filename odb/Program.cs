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

        private static void BetterWrite(string text, string linePrefix = "")
        {
            //Console.CursorLeft = 0;
            int prevTop = Console.CursorTop;
            int prevCol = Console.CursorLeft;
            Console.CursorTop = CurrentLine;
            Console.CursorLeft = CurrentColumn;
            if (linePrefix?.Length > 0)
            {
                if (CurrentColumn == 0)
                    text = linePrefix + text;
                text = text.Replace("\n", "\n" + linePrefix);
                if (text.EndsWith(linePrefix))
                    text = text.Substring(0, text.Length - linePrefix.Length);
            }
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

        private class Command
        {
            public delegate bool RunCallback(string[] args); // return true to exit
            public string Text { get; }
            public RunCallback Run { get; }
            public string Usage { get; }
            public string Description { get; }

            public Command(string text, RunCallback run, string usage, string description)
            {
                this.Text = text;
                this.Run = run;
                this.Usage = usage;
                this.Description = description;
            }
        }
        private static Dictionary<string, Command> Commands = new Dictionary<string, Command>();
        private static void RegisterCommand(string text, Command.RunCallback run, string usage, string description)
        {
            Commands.Add(text, new Command(text, run, usage, description));
        }

        private static bool Help(string[] args)
        {
            BetterWriteLine("ODB Usage:");
            List<string> commands = new List<string>(Commands.Keys);
            commands.Sort();
            foreach (string command in commands)
            {
                Command c = Commands[command];
                BetterWriteLine("\n" + c.Usage + ":");
                BetterWriteLine("   " + c.Description);
            }
            return false;
        }

        private static bool Disconnect(string[] args)
        {
            Connection.Disconnect().Wait();
            return true;
        }

        private static bool Break(string[] args)
        {
            Connection.Break();
            return false;
        }

        private static bool Step(string[] args)
        {
            // TODO
            return false;
        }

        private static bool Resume(string[] args)
        {
            Connection.Resume();
            return false;
        }

        private static bool Stack(string[] args)
        {
            // TODO
            return false;
        }

        async static Task Run(string[] args)
        {
            RegisterCommand("help", Help, "help", "Shows the list of available commands (this).");
            RegisterCommand("disconnect", Disconnect, "disconnect", "Disconnects from the debug server.");
            RegisterCommand("break", Break, "break", "Suspends execution before the next OSI instruction to be executed.");
            RegisterCommand("step", Step, "step [over|in|out]", "Executes the next OSI instruction. (over: until the next instruction in this subroutine, out: until the next instruction after this subroutine returns; default is over)");
            RegisterCommand("resume", Resume, "resume", "Resumes OSI execution when the debug server is suspended.");
            RegisterCommand("stack", Stack, "stack [values|trace|frames]", "Shows the contents of the stack of the OSI virtual machine (values: just the OSI values on the stack, trace: just the call stack, frames: the full detail of the frames of the stack; default is values)");

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
                    BetterWrite(output, "[VM]: ");
                };
                Connection.ServerException += (_, output) =>
                {
                    BetterWrite(output, "[VM Error]: ");
                };
                Connection.VMState.ExecutionStateChanged += (_, __) =>
                {
                    int oldrow = Console.CursorTop;
                    int oldcol = Console.CursorLeft;
                    Console.CursorLeft = Console.WindowWidth - 12;
                    Console.CursorTop = 0;
                    if (Connection.VMState.ExecutionState == VMState.VMExecutionState.Unknown)
                    {
                        Console.Write("?");
                        Console.Write("           ");
                    }
                    else if (Connection.VMState.ExecutionState == VMState.VMExecutionState.NativeCode)
                    {
                        Console.Write("N");
                        Console.Write("           ");
                    }
                    else if (Connection.VMState.ExecutionState == VMState.VMExecutionState.OSIRunning)
                    {
                        Console.Write("R");
                        Console.Write("           ");
                    }
                    else if (Connection.VMState.ExecutionState == VMState.VMExecutionState.OSISuspended)
                    {
                        Console.Write("P");
                        Console.Write(":0x" + Connection.VMState.InstructionPointer.ToString("X8"));
                    }
                    Console.CursorLeft = oldcol;
                    Console.CursorTop = oldrow;
                };
                
                while (!exit)
                {
                    string prologue = hostname + ":" + port + " > ";
                    Console.CursorTop = 0;
                    Console.CursorLeft = 0;
                    Console.Write(prologue.PadRight(Console.BufferWidth - 12));
                    //Console.CursorTop = 0;
                    Console.CursorLeft = prologue.Length;
                    string command = Console.ReadLine();
                    /*Console.Write(hostname + ":" + port + " > ");
                    Console.CursorTop = 0;
                    Console.CursorLeft = 0;
                    
                    Console.CursorTop = 0;
                    Console.CursorLeft = 0;
                    Console.Write(hostname + ":" + port + " > ");*/

                    string[] parts = command.Split(' ');
                    if (parts.Length > 0)
                    {
                        BetterWriteLine(" > " + command);
                        if (Commands.ContainsKey(parts[0]))
                        {
                            exit = Commands[parts[0]].Run(parts);
                        }
                        else
                        {
                            Help(new string[] { });
                        }
                    }
                }
            }
            BetterWriteLine("Press any key to exit.");
            Console.ReadKey();
        }
    }
}
