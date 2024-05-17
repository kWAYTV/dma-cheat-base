#include <Pch.h>

bool main()
{
    SetConsoleTitleA("Console - Debug");

    spdlog::set_level(spdlog::level::trace);

    if (!Mem.Init(Config::Proc::Name))
    {
        ERROR("Failed to initialize DMA");
        return 1;
    }

    Globals::ClientBase = Mem.GetBaseAddress(Config::Proc::Base);
    Mem.FixCr3();

    if (Kmbox.InitDevice(Config::Kmbox::Ip, Config::Kmbox::Port, Config::Kmbox::Uuid) != 0)
    {
        ERROR("Failed to initialize KMBOX");
        return 1;
    }

    system("pause");
    return 0;
}
