#include "directory.h"

Directory::Directory(QString name)
{
    dirname = name;
    parent = nullptr;
}

Directory::~Directory()
{
    for(Directory* d : childDirs)
    {
        delete d;
    }
    childDirs.clear();
}
