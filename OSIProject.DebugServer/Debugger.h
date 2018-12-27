#pragma once

#include "VMInterface.h"

namespace Debugger {
	// Creates the debugger thread and begins listening for TCP connections on the given port.
	void Launch(const int& listenPort);

	// Blocks until a connection is established, or the timeout is achieved. 
	bool WaitForConnection(const int& timeoutMs);

	// Called when the game is shutting down, and the debugger may or may not be running.
	void Shutdown();
}