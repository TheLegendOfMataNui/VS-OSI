#pragma once

#include "Event.h"

#include <stdarg.h>

namespace VMInterface {

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

	extern Event<VMEventArgs> VMOnExecutionBegin;
	extern Event<VMEventArgs> VMOnDebugEnabled;
	extern Event<VMEventArgs> VMOnDebugDisabled;
	extern Event<VMMessageArgs> VMOnMessage;
	extern Event<VMMessageArgs> VMOnError;

	void InstallHooks(void);
	void UninstallHooks(void);
};