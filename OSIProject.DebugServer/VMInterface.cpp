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

struct ScOSIArray {
	SxReferenceCountable super;
	DWORD unknown_vector_count_copy;
	DWORD vector_count;
	ScOSIVariant* data;
};

struct __declspec(align(4)) ScOSIObject
{
	/*SxReferenceCountable super;
	__OSIVariantVectorData vecdata;*/
	ScOSIArray super;
	WORD index;
	BYTE* osi_class_structure_ptr;
	ScOSIScript* osi_script;
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

	void NearestSubroutineName(ScOSIVirtualMachine* vm, const size_t bytecode_ptr, char* outputBuffer, int outputBufferSize) {

		char* closest_name1 = (char*)"(unknown subroutine)";
		size_t closest_name1_length = strlen(closest_name1);
		char* closest_name2 = (char*)"";
		size_t closest_name2_length = strlen(closest_name2);
		size_t closest_distance = bytecode_ptr;

		// Functions
		unsigned char* pdata = vm->osi_script->function_data_ptr;
		for (int i = 0; i < vm->osi_script->function_count; i++) {
			unsigned char name_length = *pdata;
			pdata += 1;
			char* name = (char*)pdata;
			pdata += name_length;
			size_t offset = *(size_t*)pdata + (size_t)vm->osi_script->osi_data_ptr;
			pdata += 6;

			if (offset < bytecode_ptr) {
				size_t distance = bytecode_ptr - offset;
				if (distance < closest_distance) {
					closest_distance = distance;
					closest_name1 = name;
					closest_name1_length = name_length;
				}
			}
		}

		// Methods
		for (int i = 0; i < vm->osi_script->class_count; i++) {
			pdata = vm->osi_script->class_table_entry_name_ptr[i];
			unsigned char name_length = *pdata;
			pdata += 1;
			char* name = (char*)pdata;

			pdata = vm->osi_script->class_table_entry_structure_ptr_array[i];
			unsigned char propertyCount = *pdata;
			pdata += 1 + propertyCount * 2;
			unsigned char methodCount = *pdata;
			pdata += 1;
			for (int j = 0; j < methodCount; j++) {
				unsigned short nameSymbol = *(unsigned short*)pdata;
				pdata += 2;
				size_t offset = *(size_t*)pdata + (size_t)vm->osi_script->osi_data_ptr;
				pdata += 4;

				if (offset < bytecode_ptr) {
					size_t distance = bytecode_ptr - offset;
					if (distance < closest_distance) {
						closest_distance = distance;
						closest_name1 = name;
						closest_name1_length = name_length;
						closest_name2_length = *vm->osi_script->symbol_ptr_array[nameSymbol];
						closest_name2 = (char*)(vm->osi_script->symbol_ptr_array[nameSymbol] + 1);
					}
				}
			}
		}

		if (closest_name2_length > 0) {
			sprintf_s(outputBuffer, outputBufferSize, "%.*s.%.*s", closest_name1_length, closest_name1, closest_name2_length, closest_name2);
		}
		else {
			sprintf_s(outputBuffer, outputBufferSize, "%.*s", closest_name1_length, closest_name1);
		}
	}

	void OutputDebugVariant(const ScOSIVariant& variant, ScOSIVirtualMachine* vm) {
		wchar_t data[100];

		if (variant.TypeID == Native::VARIANT_OBJECT) {
			unsigned short classIndex = ((ScOSIObject*)variant.Payload)->index;
			unsigned char classNameLength = *vm->osi_script->class_table_entry_name_ptr[classIndex];
			char* className = (char*)(vm->osi_script->class_table_entry_name_ptr[classIndex] + 1);
			swprintf_s(data, L"       [ %.*S Instance ]\n", classNameLength, className);
		}
		else if (variant.TypeID == Native::VARIANT_RETURN_ADDRESS) {
			size_t offset = variant.Payload - (size_t)vm->osi_script->osi_data_ptr;
			char name[100];
			NearestSubroutineName(vm, variant.Payload, name, 100);
			swprintf_s(data, L"\n   [ 0x%08x \"%S\" ]\n", offset, name);
		}
		else if (variant.TypeID == Native::VARIANT_ARRAY) {
			swprintf_s(data, L"       [ Array[%d] ]\n", ((ScOSIArray*)variant.Payload)->vector_count);
		}
		else if (variant.TypeID == Native::VARIANT_NULL) {
			swprintf_s(data, L"       [ Null ]\n");
		}
		else if (variant.TypeID == Native::VARIANT_INTEGER) {
			swprintf_s(data, L"       [ %d (Integer) ]\n", *(int*)& variant.Payload);
		}
		else if (variant.TypeID == Native::VARIANT_STRINGCONSTANT) {
			char* contents = vm->osi_script->string_ptr_array[variant.Payload] + 1;
			swprintf_s(data, L"       [ \"%S\" (Constant String) ]\n", contents);
		}
		else if (variant.TypeID == Native::VARIANT_FLOAT) {
			swprintf_s(data, L"       [ %f (Float) ]\n", *(float*)& variant.Payload);
		}
		else if (variant.TypeID == Native::VARIANT_STRINGTABLE) {
			swprintf_s(data, L"       [ ??? (Table String) ]\n");
		}
		else if (variant.TypeID == Native::VARIANT_COLOR5551) {
			swprintf_s(data, L"       [ ??? (5551 Color) ]\n");
		}
		else if (variant.TypeID == Native::VARIANT_COLOR8888) {
			swprintf_s(data, L"       [ ??? (Color) ]\n");
		}
		else if (variant.TypeID >= Native::VARIANT_NATIVECLASSBEGIN
			&& variant.TypeID <= Native::VARIANT_NATIVECLASSEND) {
			swprintf_s(data, L"       [ ??? (0x%x: Native Instance) ]\n", variant.TypeID);

		}
		else if (variant.TypeID == 255) {
			swprintf_s(data, L"   [ Bottom of Stack ]\n");
		}
		else {
			swprintf_s(data, L"       [ ??? (0x%x: ???) ]\n", variant.TypeID);
		}

		OutputDebugStringW(data);
	}

	void OutputDebugStack(ScOSIVirtualMachine* vm) {
		char current_subroutine[100];
		char sub_name[100];
		NearestSubroutineName(vm, (size_t)vm->bytecode_ptr, sub_name, 100);
		sprintf_s(current_subroutine, "    [ 0x%08x \"%s\" (Current Subroutine) ]\n", (size_t)vm->bytecode_ptr - (size_t)vm->osi_script->osi_data_ptr, sub_name);
		OutputDebugStringA(current_subroutine);
		for (ScOSIVariant* variant = vm->osi_stack.stack_ptr; (size_t)variant >= (size_t)& vm->osi_stack.stack; variant--) {
			OutputDebugVariant(*variant, vm);
		}
	}

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
        OutputDebugStack((ScOSIVirtualMachine*)_this);
		return tScOSIVirtualMachine__Error(_this, format, args);
	}

	// ScOSIVirtualMachine::Run
#define CUSTOM_CORE
//#define SETSTATE
#define CATCH_OSI
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
						OutputDebugStringW(L"VM CRASH:\n");
                        OutputDebugStack(_this);
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

	Native::ScOSIVariant* __cdecl lsdebugger_printstack(Native::ScOSIVariant* ret, ScOSIVirtualMachine* vm, void* param1, void* param2, void* param3, void* param4, void* param5, void* param6, void* param7, void* param8, void* param9, void* param10) {
		OutputDebugStack(vm);

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
		Native::_ScBaseString printStack = Native::_ScBaseString("printstack");
		pScOSISystem__RegisterFunction(*ScGlobalOSISystem__theOSISystem, &ns, &printStack, lsdebugger_printstack, 0, 0, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL, Native::VARIANT_NULL);
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