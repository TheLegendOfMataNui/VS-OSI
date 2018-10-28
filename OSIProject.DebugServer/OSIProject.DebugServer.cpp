// OSIProject.DebugServer.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "HookMod.h"

class DebugServerMod : public LOMNHook::HookMod {
public:
	std::wstring GetName() const override {
		return L"Litestone OSI Debugger Server";
	}

	int GetVersion() const override {
		return 0;
	}
};

DebugServerMod Instance;

extern "C" {
	__declspec(dllexport) LOMNHook::HookMod* HookmodInit() {
		return &Instance;
	}
}