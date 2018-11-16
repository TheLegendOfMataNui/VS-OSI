#pragma once

#include <WinSock2.h>

#include <stdint.h>

// Utility library for reading and writing data with a socket.
// Returns 0 for end of data, > 0 for success, and < 0 for error.

namespace SocketIO {
	// 1 byte
	int ReadUInt8(const SOCKET& socket, uint8_t& result);

	// 2 bytes, little-endian
	int ReadUInt16(const SOCKET& socket, uint16_t& result);

	// 4 bytes, little-endian
	int ReadUInt32(const SOCKET& socket, uint32_t& result);

	// 1 byte, 0 = false, > 0 = true
	int ReadBool(const SOCKET& socket, bool& result);

	// length bytes utf-8 data
	int ReadString(const SOCKET& socket, const uint16_t& length, uint8_t*& data);

	int WriteUInt8(const SOCKET& socket, const uint8_t& value);

	int WriteUInt16(const SOCKET& socket, const uint16_t& value);

	int WriteUInt32(const SOCKET& socket, const uint32_t& value);

	int WriteBool(const SOCKET& socket, const bool& value);

	int WriteString(const SOCKET& socket, const uint16_t& length, uint8_t* const& data);
}