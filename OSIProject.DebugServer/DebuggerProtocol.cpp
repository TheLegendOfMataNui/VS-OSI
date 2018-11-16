#include "stdafx.h"

#include "DebuggerProtocol.h"
#include "SocketIO.h"

using namespace Debugger::Protocol;

ClientConnectionPayload::ClientConnectionPayload(const uint8_t& versionMajor, const uint8_t& versionMinor) : VersionMajor(versionMajor), VersionMinor(versionMinor) {

}

ClientConnectionPayload::ClientConnectionPayload(const SOCKET& socket) {
	SocketIO::ReadUInt8(socket, this->VersionMajor);
	SocketIO::ReadUInt8(socket, this->VersionMinor);
}

int ClientConnectionPayload::Write(const SOCKET& socket) const {
	int status = SocketIO::WriteUInt8(socket, this->VersionMajor);
	if (status <= 0)
		return status;
	return SocketIO::WriteUInt8(socket, this->VersionMinor);
}

uint16_t ClientConnectionPayload::CalcLength() const {
	return 2;
}

ServerDebugOutputPayload::ServerDebugOutputPayload(const uint16_t& outputLength, uint8_t* const& outputData) : OutputLength(outputLength) {
	this->OutputData = new uint8_t[outputLength];
	for (uint16_t i = 0; i < outputLength; i++)
		this->OutputData[i] = outputData[i];
}

ServerDebugOutputPayload::ServerDebugOutputPayload(const SOCKET& socket) {
	SocketIO::ReadUInt16(socket, this->OutputLength);
	this->OutputData = new uint8_t[this->OutputLength];
	SocketIO::ReadString(socket, this->OutputLength, this->OutputData);
}

ServerDebugOutputPayload::~ServerDebugOutputPayload() {
	delete[] this->OutputData;
	this->OutputData = nullptr;
}

int ServerDebugOutputPayload::Write(const SOCKET& socket) const {
	int status = SocketIO::WriteUInt16(socket, this->OutputLength);
	if (status <= 0)
		return status;
	return SocketIO::WriteString(socket, this->OutputLength, this->OutputData);
}

uint16_t ServerDebugOutputPayload::CalcLength() const {
	return 2 + this->OutputLength;
}