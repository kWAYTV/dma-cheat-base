#pragma once

#define GAME_NAME "cs2.exe"
#define CLIENT_DLL "client.dll"

#define DISABLE_TIMERS 1
#define USE_LOCALHOST 1

inline const Vector3 Screen = {
	GetSystemMetrics(SM_CXSCREEN),
	GetSystemMetrics(SM_CYSCREEN)
};

inline const Vector3 ScreenCenter = {
	Screen.x / 2.f,
	Screen.y / 2.f
};

namespace Globals
{
	inline bool Running = false;
	inline uint64_t ClientBase = NULL;

	inline Matrix ViewMatrix;
};

namespace ProcInfo
{
	inline bool DmaInitialized = NULL;
	inline bool KmboxInitialized = NULL;
};
