#include "Pch.hpp"
#include "DMA.hpp"

bool DMA::Init()
{
    if (!mem.Init(GAME_NAME))
    {
        LOG_ERROR("Failed to initialize DMA");
        return 1;
    }

    Globals::ClientBase = mem.GetBaseDaddy(CLIENT_DLL);
    if (!Globals::ClientBase || Globals::ClientBase == NULL)
    {
        LOG_ERROR("Failed to get ClientBase");
        return false;
    }

    if (!mem.GetKeyboard()->InitKeyboard())
    {
        LOG_ERROR("Failed to initialize DMA Keyboard");
        return 1;
    }

    if (!mem.FixCr3())
    {
        LOG_ERROR("Failed to fix CR3");
		return false;
    }

    ProcInfo::DmaInitialized = true;

}
