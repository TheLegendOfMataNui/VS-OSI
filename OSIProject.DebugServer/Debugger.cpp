#include "stdafx.h"

#include "Debugger.h"
#include "DebuggerProtocol.h"
#include "VMInterface.h"
#include "SocketIO.h"

#include <string>

#include <WinSock2.h>
#include <iphlpapi.h>
#include <WS2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

DWORD WINAPI DebugThread(void* data);
HANDLE ThreadHandle = NULL;
DWORD ThreadID = NULL;

HANDLE LaunchedEvent = NULL;
HANDLE ConnectedEvent = NULL;
HANDLE ShutdownEvent = NULL;
HANDLE RemoteSocketMutex = NULL;
HANDLE ListenSocketMutex = NULL;

int DebugEnginePort = -1;
// Protected by RemoteSocketMutex
SOCKET RemoteSocket = INVALID_SOCKET;
// Protected by ListenSocketMutex
SOCKET ListenSocket = INVALID_SOCKET;
// Debugger thread only
bool DisconnectRemoteClient = false;
// Debugger thread only
bool ClientAuthorized = false;

WSAData WinsockData;

EventHandler<VMInterface::VMMessageArgs>* VMOnMessageHandler = nullptr;

void OnVMOnMessage(const VMInterface::VMMessageArgs&);

void Debugger::Launch(const int& port) {
	DebugEnginePort = port;

	// Hook the VM
	VMOnMessageHandler = VMInterface::VMOnMessage.AddHandler(OnVMOnMessage);

	// Create the sync events
	LaunchedEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
	ConnectedEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
	ShutdownEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
	RemoteSocketMutex = CreateMutexA(NULL, FALSE, NULL);
	ListenSocketMutex = CreateMutexA(NULL, FALSE, NULL);

	// Launch the debugger thread
	ThreadHandle = CreateThread(NULL, 0, DebugThread, nullptr, 0, &ThreadID);

	// Wait for debugger launch to complete
	DWORD wait = WaitForSingleObject(LaunchedEvent, INFINITE);
	if (wait == WAIT_OBJECT_0) {
		// we are good to go
	}
	else {
		// o no
		OutputDebugStringW(L"Wait failure launching debugger!");
	}
}

bool Debugger::WaitForConnection(const int& timeoutMs) {
	// TODO: Actually wait for the connection
	return false;
}

void Debugger::Shutdown() {
	if (ThreadHandle == NULL) {
		return;
	}

	// Tell the thread to shut down
	SetEvent(ShutdownEvent);

	// Interrupt any pending network operations
	if (WaitForSingleObject(ListenSocketMutex, INFINITE) == WAIT_OBJECT_0) {
		closesocket(ListenSocket);
		ListenSocket = INVALID_SOCKET;
		ReleaseMutex(ListenSocketMutex);
	}

	// Wait for the debugger thread to exit
	DWORD waitResult = WaitForSingleObject(ThreadHandle, INFINITE);
	ThreadHandle = NULL;

	// Clean up the sync objects
	CloseHandle(LaunchedEvent);
	LaunchedEvent = NULL;
	CloseHandle(ShutdownEvent);
	ShutdownEvent = NULL;
	CloseHandle(ConnectedEvent);
	ConnectedEvent = NULL;
	CloseHandle(RemoteSocketMutex);
	RemoteSocketMutex = NULL;
	CloseHandle(ListenSocketMutex);
	ListenSocketMutex = NULL;

	// Unhook from VM
	VMInterface::VMOnMessage.RemoveHandler(VMOnMessageHandler);
}

bool WantsExit() {
	// Wait for a shutdown with zero timeout - that is, it will instantly time out if the shutdown event is not set.
	DWORD wait = WaitForSingleObject(ShutdownEvent, 0);
	if (wait == WAIT_OBJECT_0) {
		// The shutdown event was signaled.
		return true;
	}
	else if (wait == WAIT_TIMEOUT) {
		return false;
	}
	else {
		OutputDebugStringW(L"Wait failure in Debugger::WantsExit()\n");
		return false;
	}
}

using namespace Debugger::Protocol;
void SendPacketHeader(const PacketHeader& header) {
	if (WaitForSingleObject(RemoteSocketMutex, INFINITE) == WAIT_OBJECT_0) {
		if (RemoteSocket != INVALID_SOCKET) {
			SocketIO::WriteUInt16(RemoteSocket, header.PayloadLength);
			SocketIO::WriteUInt16(RemoteSocket, header.Type);
		}
		ReleaseMutex(RemoteSocketMutex);
	}
}

void SendPacket(const PayloadType& type, const Payload& payload) {
	if (WaitForSingleObject(RemoteSocketMutex, INFINITE) == WAIT_OBJECT_0) {
		if (RemoteSocket != INVALID_SOCKET) {
			SendPacketHeader(PacketHeader { payload.CalcLength(), type });
			payload.Write(RemoteSocket);
		}
		ReleaseMutex(RemoteSocketMutex);
	}
}

// Debugger thread only
// Called while holding 
void HandlePacket(const PayloadType& type, const SOCKET& socket) {

	if (type != PayloadType::ClientConnection && !ClientAuthorized) {
		OutputDebugStringW(L"Client unauthorized for request!");
		return;
	}

	if (type == PayloadType::Invalid) {
		OutputDebugStringW(L"Invalid packet payload type!");
		DebugBreak();
	}
	else if (type == PayloadType::ClientConnection) {
		ClientConnectionPayload payloadData(socket);
		// Validate the connection request
		if (payloadData.VersionMajor != VersionMajor || payloadData.VersionMinor != VersionMinor) {
			SendPacketHeader(PacketHeader { 0, PayloadType::ServerDisconnect });
		}
		else {
			ClientAuthorized = true;
		}
	}
	else if (type == PayloadType::ClientExit) {
		// TODO: Begin the game shutdown process.
		OutputDebugStringW(L"Remote exit is not yet supported.");
	}
	else if (type == PayloadType::ClientBreak) {
		// TODO: Pause the VM
	}
	else if (type == PayloadType::ClientStep) {
		// TODO: Step the VM
	}
	else if (type == PayloadType::ClientResume) {
		// TODO: Resume the VM
	}
}

DWORD WINAPI DebugThread(void* data) {
	OutputDebugStringW(L"Debug thread starting.\n");

	// Set up winsock
	int setup = WSAStartup(MAKEWORD(2, 2), &WinsockData);
	if (setup != 0) {
		OutputDebugStringW(L"Failed to initialize Winsock!");
		DebugBreak();
	}
	addrinfo* pAddressInfo = nullptr;
	addrinfo hints = { };
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	int search = getaddrinfo(NULL, std::to_string(DebugEnginePort).c_str(), &hints, &pAddressInfo);
	if (search != 0) {
		OutputDebugStringW(L"Failed to get info for listen address!");
		DebugBreak();
	}
	if (WaitForSingleObject(ListenSocketMutex, INFINITE) == WAIT_OBJECT_0) {
		ListenSocket = socket(pAddressInfo->ai_family, pAddressInfo->ai_socktype, pAddressInfo->ai_protocol);
		if (ListenSocket == INVALID_SOCKET) {
			OutputDebugStringW(L"Failed to create socket for debugger connection!");
			DebugBreak();
		}
		int bindResult = bind(ListenSocket, pAddressInfo->ai_addr, (int)pAddressInfo->ai_addrlen);
		if (bindResult == SOCKET_ERROR) {
			OutputDebugStringW(L"Failed to bind the socket for debugger connection!");
			DebugBreak();
		}
		ReleaseMutex(ListenSocketMutex);
	}
	else {
		OutputDebugStringW(L"Failed to wait for ListenSocketMutex!");
	}
	//SOCKET listenSocket = INVALID_SOCKET;
	freeaddrinfo(pAddressInfo);

	// Launch is complete
	SetEvent(LaunchedEvent);

	while (!WantsExit()) {
		// Wait for a connection
		OutputDebugStringW(L"Waiting for connection...");
		//SOCKET remoteSocket;
		ClientAuthorized = false;

		// Use a temp copy of ListenSocket so that we can be interrupted during the listen() call by closesocket(ListenSocket)
		SOCKET listenSocketTemp = INVALID_SOCKET;
		if (WaitForSingleObject(ListenSocketMutex, INFINITE) == WAIT_OBJECT_0) {
			listenSocketTemp = ListenSocket;
			ReleaseMutex(ListenSocketMutex);
		}
		if (listen(listenSocketTemp, 1) == SOCKET_ERROR) {
			// Tell the thread to shut down
			SetEvent(ShutdownEvent);
		}
		else {
			// Use a temp copy of ListenSocket so that we can be interrupted during the listen() call by closesocket(ListenSocket)
			listenSocketTemp = INVALID_SOCKET;
			if (WaitForSingleObject(ListenSocketMutex, INFINITE) == WAIT_OBJECT_0) {
				listenSocketTemp = ListenSocket;
				ReleaseMutex(ListenSocketMutex);
			}

			SOCKET remoteSocketTemp = accept(listenSocketTemp, NULL, NULL); // We don't want to be holding RemoteSocketMutex while waiting for a connection.
			int mutex = WaitForSingleObject(RemoteSocketMutex, INFINITE);
			if (mutex == WAIT_OBJECT_0) {
				RemoteSocket = remoteSocketTemp;
				if (RemoteSocket != INVALID_SOCKET) {
					ReleaseMutex(RemoteSocketMutex);

					SetEvent(ConnectedEvent);

					// Read data from the connection while it is still open
					PacketHeader nextPacketHeader = { };
					DisconnectRemoteClient = false;
					while (!DisconnectRemoteClient) {
						int receive = -1;
						if (WaitForSingleObject(RemoteSocketMutex, INFINITE) == WAIT_OBJECT_0) {
							receive = recv(RemoteSocket, (char*)&nextPacketHeader, sizeof(PacketHeader), MSG_WAITALL);
							ReleaseMutex(RemoteSocketMutex);
						}
						else {
							continue;
						}

						if (receive > 0) {
							if (nextPacketHeader.Type == PayloadType::ClientDisconnect) {
								DisconnectRemoteClient = true;
								OutputDebugStringW(L"Client disconnecting.");
							}
							else {
								// Read and dispatch the payload

								if (WaitForSingleObject(RemoteSocketMutex, INFINITE) == WAIT_OBJECT_0) {
									HandlePacket(nextPacketHeader.Type, RemoteSocket);
									ReleaseMutex(RemoteSocketMutex);
								}

								/*unsigned char* data = nullptr;
								if (nextPacketHeader.PayloadLength > 0) {
									data = new unsigned char[nextPacketHeader.PayloadLength];
									ZeroMemory(data, nextPacketHeader.PayloadLength);
								
									if (WaitForSingleObject(RemoteSocketMutex, INFINITE) == WAIT_OBJECT_0) {
										receive = recv(RemoteSocket, (char*)data, nextPacketHeader.PayloadLength, 0);
										ReleaseMutex(RemoteSocketMutex);
									}

									if (receive == 0) {
										// Connection closed
										DisconnectRemoteClient = true;
										OutputDebugStringW(L"Connection closed by client.");
										if (data != nullptr) {
											delete[] data;
											data = nullptr;
										}
										break;
									}
									else if (receive < 0) {
										// Winsock error!
										int err = WSAGetLastError();
										OutputDebugStringW(L"Winsock receive error!");
										DebugBreak();
										if (data != nullptr) {
											delete[] data;
											data = nullptr;
										}
										continue;
									}
								}

								HandlePacket(nextPacketHeader.Type, RemoteSocket);
								if (data != nullptr) {
									delete[] data;
									data = nullptr;
								}*/
							}
						}
						else if (receive == 0) {
							// Connection closed
							DisconnectRemoteClient = true;
							OutputDebugStringW(L"Connection closed by client.");
						}
						else {
							// Winsock error!
							int err = WSAGetLastError();
							OutputDebugStringW(L"Winsock receive error!");
							DebugBreak();
						}
					}
				}
				else {
					// Failed to accept a client
					ReleaseMutex(RemoteSocketMutex);
				}
			}
			else {
				OutputDebugStringW(L"Failed to wait on RemoteSocketMutex!");
				DebugBreak();
			}
		}

		if (WaitForSingleObject(RemoteSocketMutex, INFINITE) == WAIT_OBJECT_0) {
			if (RemoteSocket != INVALID_SOCKET) {
				closesocket(RemoteSocket);
				RemoteSocket = INVALID_SOCKET;
			}
			ReleaseMutex(RemoteSocketMutex);
		}
		ResetEvent(ConnectedEvent);
	}

	OutputDebugStringW(L"Debug thread stopping.\n");

	if (WaitForSingleObject(ListenSocketMutex, INFINITE) == WAIT_OBJECT_0) {
		closesocket(ListenSocket);
		ListenSocket = INVALID_SOCKET;
	}
	WSACleanup();

	ResetEvent(LaunchedEvent);

	return 0;
}

void OnVMOnMessage(const VMInterface::VMMessageArgs& args) {
	uint8_t msg[2048];
	sprintf_s((char*)msg, 2048, args.Format, args.Data);
	uint16_t length = strlen((char*)msg);
	SendPacket(PayloadType::ServerDebugOutput, ServerDebugOutputPayload(length, msg));
}