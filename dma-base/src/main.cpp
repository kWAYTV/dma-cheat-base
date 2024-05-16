#include <Pch.h>

bool main()
{
    SetConsoleTitleA(skCrypt("Console - Debug"));

    if (!Mem.Init(Config::Proc::Name))
    {
        LOG(skCrypt("[!] Failed to initialize DMA\n"));
        return 1;
    }

    Globals::ClientBase = Mem.GetBaseAddress(Config::Proc::Base);
    Mem.FixCr3();

    if (Kmbox.InitDevice(Config::Kmbox::Ip, Config::Kmbox::Port, Config::Kmbox::Uuid) != 0)
    {
        LOG(skCrypt("[!] Failed to initialize KMBOX\n"));
        return 1;
    }

    system(skCrypt("pause"));
    return 0;
}
