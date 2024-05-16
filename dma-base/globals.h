#pragma once
#include <Pch.h>

#define DEBUG_INFO
#define USE_LOCALHOST 0

namespace globals {
	inline bool IsRunning = true;
	inline bool Localhost = true;
	inline uintptr_t ClientBase = NULL;
}