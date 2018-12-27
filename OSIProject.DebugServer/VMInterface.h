#pragma once

#include "Event.h"

#include <stdarg.h>
#include <stdint.h>

namespace VMInterface {

	enum VMExecutionState : uint8_t {
		Unknown = 0,
		NativeCode = 1,
		OSIRunning = 2,
		OSISuspended = 3,
	};

	// Base class for info about VM events
	struct VMEventArgs {
		void* VM;

		VMEventArgs(void* vm) : VM(vm) {

		}
	};

	struct VMMessageArgs : VMEventArgs {
		char* Format;
		va_list Data;

		VMMessageArgs(void* vm, char* format, va_list data) : VMEventArgs(vm), Format(format), Data(data) {

		}
	};

	struct VMStateArgs : VMEventArgs {
		VMExecutionState ExecutionState;
		unsigned int InstructionPointer;

		VMStateArgs(void* vm, VMExecutionState executionState, unsigned int instructionPointer) : VMEventArgs(vm), ExecutionState(executionState), InstructionPointer(instructionPointer) {

		}
	};

	extern Event<VMEventArgs> VMOnExecutionBegin;
	extern Event<VMEventArgs> VMOnDebugEnabled;
	extern Event<VMEventArgs> VMOnDebugDisabled;
	extern Event<VMMessageArgs> VMOnMessage;
	extern Event<VMMessageArgs> VMOnError;
	extern Event<VMStateArgs> VMOnStateChanged;

	extern HANDLE VMStateMutex; // Must hold to access VM state (including execute code)
	extern HANDLE VMExecuteEvent; // Manual reset event - VM waits on this to execute one instruction, and will clear it when running if it should step.

	void InstallHooks(void);
	void UninstallHooks(void);

	void Suspend(void);
	void Resume(void);
	void StepOne(void);
	void StepInto(void);
	void StepOut(void);
	VMExecutionState GetExecutionState();
};