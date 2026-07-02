#ifndef INODE_H
#define INODE_H
#include <QString>
#include <QDateTime>

class Inode
{
public:
    int inodeID;
    int parentID;
    bool isDir;
    QString name;
    QString content;
    int size;
    QString owner;
    QString permission;
    QDateTime createTime;
    QDateTime modifyTime;
    Inode();
};

#endif
