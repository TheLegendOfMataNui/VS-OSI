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
ScOSIVirtualMachine** GcGame__sVM = (void**)0x0083877C;
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
// TODO: GcGame__sVM, found in ScOSIVirtualMachine::call > any xref
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
	Event<VMStateArgs> VMOnStateChanged;

	HANDLE VMStateMutex;
	HANDLE VMExecuteEvent;

	VMExecutionState ExecutionState = VMExecutionState::NativeCode;

	void SetState(ScOSIVirtualMachine* _this, VMExecutionState executionState) {
		if (WaitForSingleObject(VMStateMutex, INFINITE) == WAIT_OBJECT_0) {
			ExecutionState = executionState;
			unsigned int instructionPointer = (unsigned int)*(unsigned char**)((unsigned char**)_this + 0x7D6);
			VMOnStateChanged.Invoke(VMStateArgs(_this, ExecutionState, instructionPointer));
			ReleaseMutex(VMStateMutex);
		}
	}

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
	int RunDepth = 0;
	ScOSIVirtualMachine__Run tScOSIVirtualMachine__Run = nullptr;
	void __fastcall mScOSIVirtualMachine__Run(ScOSIVirtualMachine* _this, void* unused) {
		/*RunDepth++;
		SetState(_this, VMExecutionState::OSIRunning);*/
		if (ExecutionState != VMExecutionState::OSIRunning)
			SetState(_this, VMExecutionState::OSIRunning);
		bool keepRunning = true;
		while (keepRunning) {
			// Do a quick test to see if we are suspended
			if (WaitForSingleObject(VMExecuteEvent, 0) == WAIT_TIMEOUT) {
				SetState(_this, VMExecutionState::OSISuspended);
			}
			// Wait for permission to execute the current instruction
			if (WaitForSingleObject(VMExecuteEvent, INFINITE) == WAIT_OBJECT_0) {
				// We are go, now wait for permission to use the VM
				if (WaitForSingleObject(VMStateMutex, INFINITE) == WAIT_OBJECT_0) {
					// Try not to spam packets every single instruction
					if (ExecutionState != VMExecutionState::OSIRunning)
						SetState(_this, VMExecutionState::OSIRunning);

					unsigned char op = **(unsigned char**)((unsigned char**)_this + 0x7D6);
					CodeWarriorFunctionPointer instructionHandler = ScOSIVirtualMachine__handlers[op];

					InstructionHandler handler = ScOSIVirtualMachine_vtbl[instructionHandler.OffsetIntoVtbl / 0x04];
					DWORD result = 0;
					result = handler(_this);
					keepRunning = result > 0;
					ReleaseMutex(VMStateMutex);
				}
				else {
					OutputDebugStringW(L"[ERROR]: Modded ScOSIVirtualMachine::Run: Couldn't acquire VMStateMutex!\n");
				}
			}
			else {
				OutputDebugStringW(L"[ERROR]: Modded ScOSIVirtualMachine::Run: Couldn't wait on VMExecuteEvent!\n");
			}
		}
		/*RunDepth--;
		if (RunDepth == 0)
			SetState(_this, VMExecutionState::NativeCode);*/
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
		VMStateMutex = CreateMutex(NULL, false, NULL);
		VMExecuteEvent = CreateEvent(NULL, true, true, NULL); // Manual reset event that starts TRUE, so execution may begin automatically

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
		CloseHandle(VMStateMutex);
		CloseHandle(VMExecuteEvent);
	}

	void Suspend() {
		ResetEvent(VMExecuteEvent);
	}

	void Resume() {
		SetEvent(VMExecuteEvent);
	}

	void StepOne() {
		// TODO
	}

	void StepInto() {
		// TODO
	}

	void StepOut() {
		// TODO
	}

	VMExecutionState GetExecutionState() {
		VMExecutionState result = VMExecutionState::Unknown;

		if (WaitForSingleObject(VMStateMutex, INFINITE) == WAIT_OBJECT_0) {
			result = ExecutionState;
			ReleaseMutex(VMStateMutex);
		}
		return result;
	}
}