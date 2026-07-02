#include "inode.h"

Inode::Inode()
{
    inodeID = -1;
    parentID = -1;
    isDir = false;
    size = 0;
    owner = "";
    permission = "rw-r--r--";
    createTime = QDateTime::currentDateTime();
    modifyTime = createTime;
}
