#include <Pch.h>
#include <Dma/dma.h>

int main()
{
    SetConsoleTitleA(skCrypt("Awhare - Developer Console"));

    DMA::Init();

    system(skCrypt("pause"));
    return false;
}