#ifndef DIRECTORY_H
#define DIRECTORY_H
#include<QString>
#include<QList>

class Directory
{
public:
    QString dirname;
    QList<Directory*> childDirs;
    QList<QString> files;
    Directory* parent;

    Directory(QString name);
    ~Directory();
};
#endif
