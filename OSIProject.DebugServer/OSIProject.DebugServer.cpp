// OSIProject.DebugServer.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include <LOMNAPI.h>
#include "HookMod.h"
#include "VMInterface.h"
#include "Debugger.h"
#include "ClassTracker.h"

class DebugServerMod : public LOMNHook::HookMod {
public:
	std::wstring GetName() const override {
		return L"Litestone OSI Debugger Server";
	}

	int GetVersion() const override {
		return 0;
	}

	int GetAPIRevision() const override {
		return LOMNAPI_REVISION;
	}

	void OnVMOnMessage(const VMInterface::VMMessageArgs& args) {
		char data[1000];
		sprintf_s(data, args.Format, args.Data);
		OutputDebugStringA(data);
	}

	void OnVMOnError(const VMInterface::VMMessageArgs& args) {
		char data[1000];
		sprintf_s(data, args.Format, args.Data);
		OutputDebugStringA("OSI ERROR: ");
		OutputDebugStringA(data);
	}

	void OnPreInit() override {
#if GAME_EDITION == BETA
		OutputDebugStringW(L"Litestone OSI Debugger for LOMN Beta\n");
#elif GAME_EDITION == ALPHA
		OutputDebugStringW(L"Litestone OSI Debugger for LOMN Alpha\n");
#endif

		MH_Initialize();

		ClassTracker::InstallHooks();

		VMInterface::VMOnMessage.AddHandler(&DebugServerMod::OnVMOnMessage, this);
		VMInterface::VMOnError.AddHandler(&DebugServerMod::OnVMOnError, this);
		VMInterface::InstallHooks();

		// Only start the debugger if the -debugger [port] option was given.
		int port = -1;
		for (int i = 0; i < LOMNHook::GetCommandLineArgs().size(); i++) {
			std::wstring arg = LOMNHook::GetCommandLineArgs()[i];
			if (arg == L"-debugger" && i < LOMNHook::GetCommandLineArgs().size() - 1) {
				std::wstring portArg = LOMNHook::GetCommandLineArgs()[i + 1];

				try {
					port = std::stoi(portArg);
				}
				catch (void* caught) {
					// Hey look, swallowing all errors. Generally not good practice, but hey, this just means the input was a bit wonky.
				}
			}
		}

		if (port < 0) {
			OutputDebugStringW(L"Not starting the debugger because no valid port was given.\nUse '-debugger <port>' to launch the debugger when the game starts.\n");
		}
		else {
			Debugger::Launch(port);
			Debugger::WaitForConnection(INFINITE);
		}
	}

	void OnPostDeInit() override {
		Debugger::Shutdown();
	}
};

DebugServerMod Instance;

extern "C" {
	__declspec(dllexport) LOMNHook::HookMod* HookmodInit() {
		return &Instance;
	}
}