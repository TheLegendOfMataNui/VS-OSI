#include "stdafx.h"

#include "VMInterface.h"
#include "MoreDebugDraw.h"

#include <string>
#include <Native/ScOSIVariant.h>
#include <Native/_ScBaseString.h>
#include <Native/ScIdentifier.h>

typedef void ScOSISystem;
typedef unsigned short ScOSITypeID;
//typedef void ScOSIVirtualMachine;

using namespace LOMNHook::Native;

struct __declspec(align(4)) SxReferenceCountable
{
	void *vtable;
	int count;
};

typedef ScIdentifier ScOSIToken;

struct __declspec(align(4)) ScOSIScript
{
	void *vtable;
	BYTE *osi_data_ptr;
	WORD string_count;
	BYTE *string_data_ptr;
	char **string_ptr_array;
	WORD global_count;
	BYTE *global_data_ptr;
	BYTE **global_ptr_array;
	WORD function_count;
	BYTE *function_data_ptr;
	WORD class_count;
	BYTE *class_property_data_ptr;
	BYTE **class_table_entry_structure_ptr_array;
	BYTE *class_method_data_ptr;
	BYTE **class_table_entry_name_ptr;
	WORD symbol_count;
	BYTE *symbol_data_ptr;
	BYTE **symbol_ptr_array;
	BYTE unknown[396];
	DWORD dword_468;
	DWORD dword_472;
	DWORD dword_476;
	DWORD string_table_tokens_current_size;
	ScOSIToken *string_table_tokens;
	void *source_table_data_ptr;
};

struct __declspec(align(4)) ScOSIStack
{
	ScOSIVariant stack[1000];
	WORD word_unknown;
	ScOSIVariant *stack_ptr;
};

struct __declspec(align(4)) ScOSIVirtualMachine
{
	SxReferenceCountable super;
	ScOSIScript *osi_script;
	ScOSIStack osi_stack;
	ScOSIVariant *unknown_variant_ptr_second_to_end__maybe_this;
	BYTE *bytecode_ptr;
};


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
ScOSIVirtualMachine** GcGame__sVM = (ScOSIVirtualMachine**)0x0083877C;
ScOSISystem__RegisterFunction pScOSISystem__RegisterFunction = (ScOSISystem__RegisterFunction)0x005FAB30;
ScOSIVirtualMachine__Message pScOSIVirtualMachine__Message = (ScOSIVirtualMachine__Message)0x0060BEF0;
ScOSIVirtualMachine__Error pScOSIVirtualMachine__Error = (ScOSIVirtualMachine__Error)0x0060BE80;
CodeWarriorFunctionPointer* ScOSIVirtualMachine__handlers = (CodeWarriorFunctionPointer*)0x00752768;
ScOSIVirtualMachine__Run pScOSIVirtualMachine__Run = (ScOSIVirtualMachine__Run)0x0060B850;
InstructionHandler* ScOSIVirtualMachine_vtbl = (InstructionHandler*)0x007514E4;
#elif GAME_EDITION == ALPHA
ScOSISystem** ScGlobalOSISystem__theOSISystem = (void**)0x00630CE8;
// TODO: GcGame__sVM, found in ScOSIVirtualMachine::call > any xref
ScOSISystem__RegisterFunction pScOSISystem__RegisterFunction = (ScOSISystem__RegisterFunction)0x00572F90;
ScOSIVirtualMachine__Message pScOSIVirtualMachine__Message = (ScOSIVirtualMachine__Message)0x0057D610;
ScOSIVirtualMachine__Error pScOSIVirtualMachine__Error = (ScOSIVirtualMachine__Error)0x0057D5A0;
CodeWarriorFunctionPointer* ScOSIVirtualMachine__handlers = (CodeWarriorFunctionPointer*)0x00633BB8;
ScOSIVirtualMachine__Run pScOSIVirtualMachine__Run = (ScOSIVirtualMachine__Run)0x0057CF70;
InstructionHandler* ScOSIVirtualMachine_vtbl = (InstructionHandler*)0x00632994;
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
		char locationMessage[256];
		//unsigned int instructionPointer = (unsigned int) * (unsigned char**)((unsigned char**)_this + 0x7D6);
		unsigned int currentFileOffset = (size_t)(*GcGame__sVM)->bytecode_ptr - (size_t)(*GcGame__sVM)->osi_script->osi_data_ptr;
		sprintf_s(locationMessage, "At offset 0x%08x\n", currentFileOffset);
		OutputDebugStringA(locationMessage);
		VMOnError.Invoke(VMMessageArgs(_this, format, args));
		return tScOSIVirtualMachine__Error(_this, format, args);
	}

	// ScOSIVirtualMachine::Run
#define CUSTOM_CORE
//#define SETSTATE
//#define CATCH_OSI
//#define SYNCH

#ifdef CUSTOM_CORE
	int RunDepth = 0;
	ScOSIVirtualMachine__Run tScOSIVirtualMachine__Run = nullptr;
	void __fastcall mScOSIVirtualMachine__Run(ScOSIVirtualMachine* _this, void* unused) {
		/*RunDepth++;
		SetState(_this, VMExecutionState::OSIRunning);*/
#ifdef SETSTATE
		if (ExecutionState != VMExecutionState::OSIRunning)
			SetState(_this, VMExecutionState::OSIRunning);
#endif
		bool keepRunning = true;
		while (keepRunning) {
			// Do a quick test to see if we are suspended
#ifdef SETSTATE
			if (WaitForSingleObject(VMExecuteEvent, 0) == WAIT_TIMEOUT) {
				SetState(_this, VMExecutionState::OSISuspended);
			}
#endif
			// Wait for permission to execute the current instruction
#ifdef SYNCH
			if (WaitForSingleObject(VMExecuteEvent, INFINITE) == WAIT_OBJECT_0) {
				// We are go, now wait for permission to use the VM
				if (WaitForSingleObject(VMStateMutex, INFINITE) == WAIT_OBJECT_0) {
#endif
					// Try not to spam packets every single instruction
#ifdef SETSTATE
					if (ExecutionState != VMExecutionState::OSIRunning)
						SetState(_this, VMExecutionState::OSIRunning);
#endif

					//unsigned char op = **(unsigned char**)((unsigned char**)_this + 0x7D6);
					DWORD scriptOffset = (DWORD)_this->bytecode_ptr - (DWORD)_this->osi_script->osi_data_ptr;
					unsigned char op = *_this->bytecode_ptr;
					CodeWarriorFunctionPointer instructionHandler = ScOSIVirtualMachine__handlers[op];

					InstructionHandler handler = ScOSIVirtualMachine_vtbl[instructionHandler.OffsetIntoVtbl / 0x04];
					DWORD result = 0;

#ifdef CATCH_OSI
					__try {
#endif
						result = handler(_this);
#ifdef CATCH_OSI
					}
					__except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
						char buf[255];
						sprintf_s(buf, "Exception executing OSI bytecode at offset 0x%x!!\n", scriptOffset);
						OutputDebugStringA(buf);
						DebugBreak();
					}
#endif
					keepRunning = result > 0;
#ifdef SYNCH
					ReleaseMutex(VMStateMutex);
				}
				else {
					OutputDebugStringW(L"[ERROR]: Modded ScOSIVirtualMachine::Run: Couldn't acquire VMStateMutex!\n");
				}
			}
			else {
				OutputDebugStringW(L"[ERROR]: Modded ScOSIVirtualMachine::Run: Couldn't wait on VMExecuteEvent!\n");
			}
#endif
		}
		/*RunDepth--;
		if (RunDepth == 0)
			SetState(_this, VMExecutionState::NativeCode);*/
	}
#endif

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

	Native::ScOSIVariant* __cdecl lsdebugger_settriggerplanes(Native::ScOSIVariant* ret, ScOSIVirtualMachine* vm, void* param1, void* param2, void* param3, void* param4, void* param5, void* param6, void* param7, void* param8, void* param9, void* param10) {
		*gTriggerPlanes = (int)param1 != 0;
		// Return a null variant
		ret->Payload = 0xFF;
		ret->TypeID = Native::VARIANT_NULL;
		return ret;
	}

	Native::ScOSIVariant* __cdecl lsdebugger_settriggerboxes(Native::ScOSIVariant* ret, ScOSIVirtualMachine* vm, void* param1, void* param2, void* param3, void* param4, void* param5, void* param6, void* param7, void* param8, void* param9, void* param10) {
		*gTriggerBoxes = (int)param1 != 0;
		// Return a null variant
		ret->Payload = 0xFF;
		ret->TypeID = Native::VARIANT_NULL;
		return ret;
	}

	void InstallHooks() {
		VMStateMutex = CreateMutex(NULL, false, NULL);
		VMExecuteEvent = CreateEvent(NULL, true, true, NULL); // Manual reset event that starts TRUE, so execution may begin automatically

		MDDInitialize();

		// Native function hooking
		MH_STATUS s = MH_CreateHook(pScOSIVirtualMachine__Message, &mScOSIVirtualMachine__Message, (void**)&tScOSIVritualMachine__Message);
		s = MH_CreateHook(pScOSIVirtualMachine__Error, &mScOSIVirtualMachine__Error, (void**)&tScOSIVirtualMachine__Error);
#ifdef CUSTOM_CORE
		s = MH_CreateHook(pScOSIVirtualMachine__Run, &mScOSIVirtualMachine__Run, (void**)&tScOSIVirtualMachine__Run);
#endif
		MH_EnableHook(MH_ALL_HOOKS);

		// Register OSI functions
		Native::_ScBaseString ns = Native::_ScBaseString("lsdebugger");
		Native::_ScBaseString setWireframe = Native::_ScBaseString("setwireframe");
		pScOSISystem__RegisterFunction(*ScGlobalOSISystem__theOSISystem, &ns, &setWireframe, lsdebugger_setwireframe, 1, 1, Native::VARIANT_INTEGER, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF, 0xF);
		Native::_ScBaseString setCollisionBoxes = Native::_ScBaseString("setcollisionboxes");
		pScOSISystem__RegisterFunction(*ScGlobalOSISystem__theOSISystem, &ns, &setCollisionBoxes, lsdebugger_setcollisionboxes, 1, 1, Native::VARIANT_INTEGER, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL);
		Native::_ScBaseString setTriggerPlanes = Native::_ScBaseString("settriggerplanes");
		pScOSISystem__RegisterFunction(*ScGlobalOSISystem__theOSISystem, &ns, &setTriggerPlanes, lsdebugger_settriggerplanes, 1, 1, Native::VARIANT_INTEGER, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL);
		Native::_ScBaseString setTriggerBoxes = Native::_ScBaseString("settriggerboxes");
		pScOSISystem__RegisterFunction(*ScGlobalOSISystem__theOSISystem, &ns, &setTriggerBoxes, lsdebugger_settriggerboxes, 1, 1, Native::VARIANT_INTEGER, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL);
	}

	void UninstallHooks() {
		MH_RemoveHook(pScOSIVirtualMachine__Message);
		CloseHandle(VMStateMutex);
		CloseHandle(VMExecuteEvent);
		MDDShutdown();
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