#include "pch.h"
#include "dma.h"

bool DMA::Init()
{
	if (!Mem.Init("cs2.exe"))
	{
		LOG("Failed to initialize process\n");
		return false;
	}

	globals::ClientBase = Mem.GetBaseAddress("client.dll");
	Mem.FixCr3();

	return true;
}
