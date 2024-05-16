#pragma once
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <iostream>
#include <algorithm> 
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <fstream>
#include <ppltasks.h>
#include <windowsx.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <TlHelp32.h>
#include <thread>
#include <filesystem>
#include <cctype>    
#include <iomanip>
#include <random>
#include <sstream>
#include <locale>
#include <cstdint>
#include <d2d1_1.h>
#include <dwrite.h>
#include <dwmapi.h>
#include <ShellScalingApi.h>
#include <vmmdll.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "vmm.lib")
#pragma comment(lib, "leechcore.lib")
#pragma comment(lib, "FTD3XX.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "Dwrite")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Shcore.lib")

#include "skCrypter.h"
#include "Memory.h"
#include "Kmbox/Manager.h"
#include "config.h"
#include "globals.h"

// Logging
#define DEBUG_INFO 1

#ifdef DEBUG_INFO
#define LOG(fmt, ...) std::printf(fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) std::wprintf(fmt, ##__VA_ARGS__)
#else
#define LOG
#define LOGW
#endif