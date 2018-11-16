using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;

namespace OSIProject.DebugInterop
{
    // NOTE: Keep OSIProject.DebugServer/DebuggerProtocol.h in sync with this
    public enum PayloadType : ushort
    {
        Invalid,
        ClientConnection, // From client: Connection request.
        ClientDisconnect, // From client: Disconnecting. Expect no more packets, and close connection immediately.
        ClientExit, // From client: Exit the game.
        ClientBreak, // From client: Pause the VM before the next instruction.
        ClientStep, // From client: Execute one instruction, and pause before the next instruction.
        ClientResume, // From client: Resume VM execution.
        ServerDisconnect, // From server: Disconnecting. Expect no more packets, and close connection immediately.
        ServerStateChange, // From server: State has changed (no code, running, paused).
        ServerExecutionState, // From server: Current bytecode offset, stack status.
        ServerStackState, // From server: Current stack data.
        ServerDebugOutput, // From server: Debug output from OSI.
        ServerException, // From server: Exception message and details.
    }

    public class PacketHeader
    {
        public ushort PayloadLength { get; } = 0;
        public PayloadType Type { get; } = PayloadType.Invalid;

        public PacketHeader(ushort payloadLength, PayloadType type)
        {
            this.PayloadLength = payloadLength;
            this.Type = type;
        }

        public PacketHeader(BinaryReader reader)
        {
            this.PayloadLength = reader.ReadUInt16();
            this.Type = (PayloadType)reader.ReadUInt16();
        }

        public void Write(BinaryWriter writer)
        {
            writer.Write(this.PayloadLength);
            writer.Write((ushort)this.Type);
        }
    }

    public class Packet
    {
        public PacketHeader Header { get; }

        public Packet(PacketHeader header)
        {
            this.Header = header;
        }

        public Packet(BinaryReader reader)
        {
            this.Header = new PacketHeader(reader);
        }

        public void Write(BinaryWriter writer)
        {
            this.Header.Write(writer);
        }
    }
}
