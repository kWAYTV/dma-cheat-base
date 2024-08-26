#pragma once

#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Standard Libraries
#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <locale>
#include <cstdint>
#include <cctype>
#include <chrono>
#include <thread>
#include <filesystem>
#include <regex>
#include <conio.h>

// Windows Libraries
#include <windowsx.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <TlHelp32.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <dwmapi.h>
#include <ShellScalingApi.h>

// PPL Tasks
#include <ppltasks.h>

// External Libraries & Namespace Usings
using namespace std;

#include "skCrypter.h"
#include "spdlog/spdlog.h"

#include "fnv1a/fnv1a.hpp"
using namespace ext;

#include "json.hpp"
using json = nlohmann::json;

// Linker Commands
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "Dwrite")
#pragma comment(lib, "windowscodecs.lib")
#pragma comment(lib, "Dwmapi.lib")
#pragma comment(lib, "Shcore.lib")

// Helper macro to extract file name from full path
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__))

// Logging
#define DEBUG_INFO
#ifdef DEBUG_INFO
#define LOG_INFO(fmt, ...) spdlog::info("[{}:{}] " fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) spdlog::error("[{}:{}] " fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) spdlog::warn("[{}:{}] " fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) spdlog::critical("[{}:{}] " fmt, __FILENAME__, __LINE__, ##__VA_ARGS__)
#define LOG_TESTING(fmt, ...) spdlog::warn("TESTING")
#else
#define LOG_INFO
#define LOG_ERROR
#define LOG_WARN
#define LOG_CRITICAL
#define LOG_TESTING
#endif

#define THROW_EXCEPTION
#ifdef THROW_EXCEPTION
#define THROW(fmt, ...) throw std::runtime_error(fmt, ##__VA_ARGS__)
#endif

// Includes
#include "Math/Matrix.hpp"
#include "Math/Vector.hpp"

#include "Kmbox/Manager.hpp"
#include "Config.hpp"
#include "Globals.hpp"

#include "Dma/DMA.hpp"

#include "Misc/Animation/Animation.hpp"
#include "Misc/Timer/Timer.hpp"
#include "Misc/Exception.hpp"