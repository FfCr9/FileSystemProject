#ifndef INODE_H
#define INODE_H
#include <QString>

class Inode
{
public:
    int inodeID;
    QString content;
    int size;
    Inode();
};

#endif
