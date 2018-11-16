#include "stdafx.h"

#include "SocketIO.h"

int SocketIO::ReadUInt8(const SOCKET& socket, uint8_t& result) {
	return recv(socket, (char*)&result, 1, MSG_WAITALL);
}

int SocketIO::ReadUInt16(const SOCKET& socket, uint16_t& result) {
	return recv(socket, (char*)result, 2, MSG_WAITALL);
}

int SocketIO::ReadUInt32(const SOCKET& socket, uint32_t& result) {
	return recv(socket, (char*)result, 4, MSG_WAITALL);
}

int SocketIO::ReadBool(const SOCKET& socket, bool& result) {
	uint8_t value;
	int status = ReadUInt8(socket, value);
	result = value != 0;
	return status;
}

int SocketIO::ReadString(const SOCKET& socket, const uint16_t& length, uint8_t*& data) {
	/*int status = ReadUInt16(socket, length);
	if (status <= 0)
		return status;*/
	return recv(socket, (char*)data, length, MSG_WAITALL);
}

int SocketIO::WriteUInt8(const SOCKET& socket, const uint8_t& value) {
	return send(socket, (char*)&value, 1, 0);
}

int SocketIO::WriteUInt16(const SOCKET& socket, const uint16_t& value) {
	return send(socket, (char*)&value, 2, 0);
}

int SocketIO::WriteUInt32(const SOCKET& socket, const uint32_t& value) {
	return send(socket, (char*)&value, 4, 0);
}

int SocketIO::WriteBool(const SOCKET& socket, const bool& value) {
	return WriteUInt8(socket, value ? 1 : 0);
}

int SocketIO::WriteString(const SOCKET& socket, const uint16_t& length, uint8_t* const& data) {
	/*int status = WriteUInt16(socket, length);
	if (status <= 0)
		return status;*/
	return send(socket, (char*)data, length, 0);
}