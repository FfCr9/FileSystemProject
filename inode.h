#ifndef INODE_H
#define INODE_H
#include <QString>
#include <QDateTime>

class Inode
{
public:
    int inodeID;
    QString content;
    int size;
    QString owner;
    QString permission;
    QDateTime createTime;
    Inode();
};

#endif
