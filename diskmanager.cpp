#include "diskmanager.h"

DiskManager::DiskManager()
{
    for(int i = 0; i < 100; i++)
    {
        disk[i] = 0;
    }
}

int DiskManager::allocateBlock()
{
    for(int i = 0; i < 100; i++)
    {
        if(disk[i] == 0)
        {
            disk[i] = 1;
            return i;
        }
    }
    return -1;
}

void DiskManager::freeBlock(int blockID)
{
    if(blockID >= 0 && blockID < 100)
    {
        disk[blockID] = 0;
    }
}
