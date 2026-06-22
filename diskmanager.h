#ifndef DISKMANAGER_H
#define DISKMANAGER_H

class DiskManager
{
public:
    DiskManager();
    int allocateBlock();
    void freeBlock(int blockID);
private:
    int disk[100];
};

#endif
