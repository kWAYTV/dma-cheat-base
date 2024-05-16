#include <Pch.h>

bool main()
{
    SetConsoleTitleA(skCrypt("Developer Console"));

	if (!Mem.Init(globals::ProcName)
	{
		LOG(skCrypt("Failed to initialize process\n"));
		return false;
	}

	globals::ClientBase = Mem.GetBaseAddress(globals::ProcBase);
	Mem.FixCr3();

    system(skCrypt("pause"));
    return false;
}