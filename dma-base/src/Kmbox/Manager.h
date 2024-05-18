#pragma once

#include "Config.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <Windows.h>
#include <math.h>
#include <random>
#include <string>
#include <thread>

class KmBoxMouse
{
public:
    soft_mouse_t MouseData{};
public:
    // Move
    int Move(int x, int y);
    // Auto move
    int Move_Auto(int x, int y, int Runtime);
    // Left button
    int Left(bool Down);
    // Right button
    int Right(bool Down);
    // Middle button
    int Middle(bool Down);
};

class KmBoxKeyBoard
{
public:
    std::thread t_Listen;
    WORD MonitorPort;
    SOCKET s_ListenSocket = 0;
    bool ListenerRuned = false;
public:
    standard_keyboard_report_t hw_Keyboard;
    standard_mouse_report_t hw_Mouse;
public:
    ~KmBoxKeyBoard();
    void ListenThread();
    int StartMonitor(WORD Port);
    void EndMonitor();
public:
    bool GetKeyState(WORD vKey);
};

class KmBoxNetManager
{
private:
    SOCKADDR_IN AddrServer;
    SOCKET s_Client = 0;
    client_data ReceiveData;
    client_data PostData;
private:
    int NetHandler();
    int SendData(int DataLength);
public:
    ~KmBoxNetManager();
    // Initialize device
    int InitDevice(const std::string& IP, WORD Port, const std::string& Mac);
    // Reboot device
    int RebootDevice();
    // Set device config
    int SetConfig(const std::string& IP, WORD Port);
	// Speed test
	void SpeedTest(int count = 1000);

    // Fill LCD with color
    int FillLCDColor(unsigned short rgb565);
    // Change picture
    int ChangePicture(const unsigned char* buff_128_160);
    // Change bottom picture
    int ChangePictureBottom(const unsigned char* buff_128_80);
public:
    friend class KmBoxMouse;
    KmBoxMouse Mouse;
    friend class KmBoxKeyBoard;
    KmBoxKeyBoard KeyBoard;
};

inline KmBoxNetManager Kmbox;
