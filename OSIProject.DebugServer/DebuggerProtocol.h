#pragma once

#include "SocketIO.h"

#include <stdint.h>

namespace Debugger {
	namespace Protocol {
		const int VersionMajor = 0;
		const int VersionMinor = 1;

		// NOTE: Keep OSIProject.DebugInterop/PacketHeader.cs in sync with this
		enum PayloadType : uint16_t {
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
		};

		struct PacketHeader {
			uint16_t PayloadLength = 0;
			PayloadType Type = Invalid;
		};

		class Payload {
		public:
			// Called from within RemoteSocketMutex
			virtual int Write(const SOCKET& socket) const = 0;
			// Called from within RemoteSocketMutex
			virtual uint16_t CalcLength() const = 0;
		};

		class ClientConnectionPayload : public Payload {
		public:
			uint8_t VersionMajor;
			uint8_t VersionMinor;
			ClientConnectionPayload(const uint8_t& versionMajor, const uint8_t& versionMinor);
			ClientConnectionPayload(const SOCKET& socket);
			int Write(const SOCKET& socket) const override;
			uint16_t CalcLength() const override;
		};

		class ServerDebugOutputPayload : public Payload {
		public:
			uint16_t OutputLength;
			uint8_t* OutputData; // Created in constructor, freed in destructor
			ServerDebugOutputPayload(const uint16_t& outputLength, uint8_t* const& outputData);
			ServerDebugOutputPayload(const SOCKET& socket);
			~ServerDebugOutputPayload();
			int Write(const SOCKET& socket) const override;
			uint16_t CalcLength() const override;
		};
	}
}

