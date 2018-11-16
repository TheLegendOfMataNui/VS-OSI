#include "stdafx.h"

#include "VMInterface.h"

#include <string>
#include <Native/ScOSIVariant.h>
#include <Native/_ScBaseString.h>

typedef void ScOSISystem;
typedef unsigned short ScOSITypeID;
typedef void ScOSIVirtualMachine;

using namespace LOMNHook;

typedef Native::ScOSIVariant* (*OSIFunctionCallback)(Native::ScOSIVariant*, ScOSIVirtualMachine*, void*, void*, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void(__thiscall *ScOSISystem__RegisterFunction)(ScOSISystem*, Native::_ScBaseString*, Native::_ScBaseString*, OSIFunctionCallback, char, char, ScOSITypeID, ScOSITypeID, ScOSITypeID, ScOSITypeID, ScOSITypeID, ScOSITypeID, ScOSITypeID, ScOSITypeID, ScOSITypeID, ScOSITypeID);
typedef int(*ScOSIVirtualMachine__Message)(void*, char*, va_list);
typedef int(*ScOSIVirtualMachine__Error)(void*, char*, va_list);
typedef bool(__thiscall *InstructionHandler)(ScOSIVirtualMachine*);
typedef void(__fastcall *ScOSIVirtualMachine__Run)(void*, void*);

struct CodeWarriorFunctionPointer {
	DWORD ClassOffset;
	DWORD OffsetIntoVtbl; // or -1 for static method
	DWORD pVtblOffsetOrStaticAddress;
};

#if GAME_EDITION == BETA
ScOSISystem** ScGlobalOSISystem__theOSISystem = (void**)0x0074D644;
ScOSISystem__RegisterFunction pScOSISystem__RegisterFunction = (ScOSISystem__RegisterFunction)0x005FAB30;
ScOSIVirtualMachine__Message pScOSIVirtualMachine__Message = (ScOSIVirtualMachine__Message)0x0060BEF0;
ScOSIVirtualMachine__Error pScOSIVirtualMachine__Error = (ScOSIVirtualMachine__Error)0x0060BE80;
CodeWarriorFunctionPointer* ScOSIVirtualMachine__handlers = (CodeWarriorFunctionPointer*)0x00752768;
ScOSIVirtualMachine__Run pScOSIVirtualMachine__Run = (ScOSIVirtualMachine__Run)0x0060B850;
InstructionHandler* ScOSIVirtualMachine_vtbl = (InstructionHandler*)0x007514E4;
bool* GcDebugOptions__sWireframe = (bool*)0x705CA8;
bool* gCollisionBoxes = (bool*)0x705CAC;
#elif GAME_EDITION == ALPHA
ScOSISystem** ScGlobalOSISystem__theOSISystem = (void**)0x00630CE8;
ScOSISystem__RegisterFunction pScOSISystem__RegisterFunction = (ScOSISystem__RegisterFunction)0x00572F90;
ScOSIVirtualMachine__Message pScOSIVirtualMachine__Message = (ScOSIVirtualMachine__Message)0x0057D610;
ScOSIVirtualMachine__Error pScOSIVirtualMachine__Error = (ScOSIVirtualMachine__Error)0x0057D5A0;
CodeWarriorFunctionPointer* ScOSIVirtualMachine__handlers = (CodeWarriorFunctionPointer*)0x00633BB8;
ScOSIVirtualMachine__Run pScOSIVirtualMachine__Run = (ScOSIVirtualMachine__Run)0x0057CF70;
InstructionHandler* ScOSIVirtualMachine_vtbl = (InstructionHandler*)0x00632994;
bool* GcDebugOptions__sWireframe = (bool*)0x00610124;
bool* gCollisionBoxes = (bool*)0x0061012C;
#endif


namespace VMInterface {
	Event<VMEventArgs> VMOnExecutionBegin;
	Event<VMEventArgs> VMOnDebugEnabled;
	Event<VMEventArgs> VMOnDebugDisabled;
	Event<VMMessageArgs> VMOnMessage;
	Event<VMMessageArgs> VMOnError;

	// ScOSIVirtualMachine::Message
	ScOSIVirtualMachine__Message tScOSIVritualMachine__Message = nullptr; // Trampouline
	int mScOSIVirtualMachine__Message(void* _this, char* format, va_list args) {
		int result = tScOSIVritualMachine__Message(_this, format, args);
		VMOnMessage.Invoke(VMMessageArgs(_this, format, args));
		return result;
	}

	// ScOSIVirtualMachine::Error
	ScOSIVirtualMachine__Error tScOSIVirtualMachine__Error = nullptr;
	int mScOSIVirtualMachine__Error(void* _this, char* format, va_list args) {
		VMOnError.Invoke(VMMessageArgs(_this, format, args));
		return tScOSIVirtualMachine__Error(_this, format, args);
	}

	// ScOSIVirtualMachine::Run
	ScOSIVirtualMachine__Run tScOSIVirtualMachine__Run = nullptr;
	void __fastcall mScOSIVirtualMachine__Run(ScOSIVirtualMachine* _this, void* unused) {
		bool keepRunning = true;
		while (keepRunning) {
			unsigned char op = **(unsigned char**)((unsigned char**)_this + 0x7D6);
			CodeWarriorFunctionPointer instructionHandler = ScOSIVirtualMachine__handlers[op];

			InstructionHandler handler = ScOSIVirtualMachine_vtbl[instructionHandler.OffsetIntoVtbl / 0x04];
			DWORD result = 0;
			result = handler(_this);
			keepRunning = result > 0;
		}
	}

	// Wireframe control
	Native::ScOSIVariant* __cdecl lsdebugger_setwireframe(Native::ScOSIVariant* ret, ScOSIVirtualMachine* vm, void* param1, void* param2, void* param3, void* param4, void* param5, void* param6, void* param7, void* param8, void* param9, void* param10) {
		*GcDebugOptions__sWireframe = (int)param1 != 0;
		// Return a null variant
		ret->Payload = 0xFF;
		ret->TypeID = Native::VARIANT_NULL;
		return ret;
	}

	Native::ScOSIVariant* __cdecl lsdebugger_setcollisionboxes(Native::ScOSIVariant* ret, ScOSIVirtualMachine* vm, void* param1, void* param2, void* param3, void* param4, void* param5, void* param6, void* param7, void* param8, void* param9, void* param10) {
		*gCollisionBoxes = (int)param1 != 0;
		// Return a null variant
		ret->Payload = 0xFF;
		ret->TypeID = Native::VARIANT_NULL;
		return ret;
	}

	void InstallHooks() {
		// Native function hooking
		MH_STATUS s = MH_CreateHook(pScOSIVirtualMachine__Message, &mScOSIVirtualMachine__Message, (void**)&tScOSIVritualMachine__Message);
		s = MH_CreateHook(pScOSIVirtualMachine__Error, &mScOSIVirtualMachine__Error, (void**)&tScOSIVirtualMachine__Error);
		s = MH_CreateHook(pScOSIVirtualMachine__Run, &mScOSIVirtualMachine__Run, (void**)&tScOSIVirtualMachine__Run);
		MH_EnableHook(MH_ALL_HOOKS);

		// Register OSI functions
		Native::_ScBaseString ns = Native::_ScBaseString("lsdebugger");
		Native::_ScBaseString setWireframe = Native::_ScBaseString("setwireframe");
		pScOSISystem__RegisterFunction(*ScGlobalOSISystem__theOSISystem, &ns, &setWireframe, lsdebugger_setwireframe, 1, 1, Native::VARIANT_INTEGER, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF);
		Native::_ScBaseString setCollisionBoxes = Native::_ScBaseString("setcollisionboxes");
		pScOSISystem__RegisterFunction(*ScGlobalOSISystem__theOSISystem, &ns, &setCollisionBoxes, lsdebugger_setcollisionboxes, 1, 1, Native::VARIANT_INTEGER, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL);
	}

	void UninstallHooks() {
		MH_RemoveHook(pScOSIVirtualMachine__Message);
	}
}