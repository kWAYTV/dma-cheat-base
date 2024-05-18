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

#include "json.hpp"
using json = nlohmann::json;

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "vmm.lib")
#pragma comment(lib, "leechcore.lib")
#pragma comment(lib, "FTD3XX.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "Dwrite")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Shcore.lib")

#include <spdlog/spdlog.h>

// Logging
//#define DEBUG_INFO
#ifdef DEBUG_INFO
#define INFO(fmt, ...) spdlog::info("[{}:{}] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define ERROR(fmt, ...) spdlog::error("[{}:{}] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define WARN(fmt, ...) spdlog::warn("[{}:{}] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#define CRITICAL(fmt, ...) spdlog::critical("[{}:{}] " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define INFO(fmt, ...) spdlog::info(fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) spdlog::error(fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) spdlog::warn(fmt, ##__VA_ARGS__)
#define CRITICAL(fmt, ...) spdlog::critical(fmt, ##__VA_ARGS__)
#endif

#include "Memory.h"
#include "Kmbox/Manager.h"
#include "Config.h"
#include "Globals.h"