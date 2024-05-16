#include <Pch.h>

bool main()
{
    SetConsoleTitleA(skCrypt("Awhare - Developer Console"));

	if (!Mem.Init("cs2.exe"))
	{
		LOG(skCrypt("Failed to initialize process\n"));
		return false;
	}

	globals::ClientBase = Mem.GetBaseAddress("client.dll");
	Mem.FixCr3();

    system(skCrypt("pause"));
    return false;
}