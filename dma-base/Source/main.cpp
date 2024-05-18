#include <Pch.h>

bool main()
{
    SetConsoleTitleA("Console - Debug");

    spdlog::set_level(spdlog::level::trace);

	if (!config.Init())
    {
		ERROR("Failed to initialize config. If you didn't have one, fill the new created file.");
		system("pause");
		return 1;
	}

    if (!Mem.Init(config.Proc.Name))
    {
        ERROR("Failed to initialize DMA");
        system("pause");
        return 1;
    }

    Globals::ClientBase = Mem.GetBaseAddress(config.Proc.Base);
    Mem.FixCr3();

    if (Kmbox.InitDevice(config.Kmbox.Ip, config.Kmbox.Port, config.Kmbox.Uuid) != 0)
    {
        ERROR("Failed to initialize KMBOX");
        system("pause");
        return 1;
    }

    system("pause");
    return 0;
}
