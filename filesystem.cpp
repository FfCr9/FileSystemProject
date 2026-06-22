#include "filesystem.h"

FileSystem::FileSystem()
{
    loadUsersFromFile();
    loadFilesFromFile();
}

bool FileSystem::registerUser(QString username, QString password)
{
    for(auto &u : users)
    {
        if(u.username == username)
            return false;
    }
    users.append(User(username, password));
    saveUsersToFile();
    return true;
}

bool FileSystem::login(QString username, QString password)
{
    for(auto &u : users)
    {
        if(u.username == username && u.password == password)
        {
            currentUser = username;
            return true;
        }
    }
    return false;
}

bool FileSystem::createFile(QString filename)
{
    if(fileMap.contains(filename))
        return false;
    int bid = diskManager.allocateBlock();
    if(bid == -1)
        return false;

    Inode *node = new Inode;
    node->inodeID = bid;
    inodeTable[bid] = node;
    fileMap[filename] = bid;
    saveFilesToFile();
    return true;
}

bool FileSystem::deleteFile(QString filename)
{
    if(!fileMap.contains(filename))
        return false;
    int bid = fileMap[filename];
    diskManager.freeBlock(bid);
    delete inodeTable[bid];
    inodeTable.remove(bid);
    fileMap.remove(filename);
    saveFilesToFile();
    return true;
}

bool FileSystem::writeFile(QString filename, QString content)
{
    if(!fileMap.contains(filename))
        return false;
    int bid = fileMap[filename];
    inodeTable[bid]->content = content;
    inodeTable[bid]->size = content.length();
    saveFilesToFile();
    return true;
}

QString FileSystem::readFile(QString filename)
{
    if(!fileMap.contains(filename))
        return "";
    int bid = fileMap[filename];
    return inodeTable[bid]->content;
}

void FileSystem::saveUsersToFile()
{
    QFile f("data/users.txt");
    if(!f.open(QIODevice::WriteOnly|QIODevice::Text)) return;
    QTextStream out(&f);
    for(auto &u : users)
    {
        out << u.username << " " << u.password << "\n";
    }
    f.close();
}

void FileSystem::loadUsersFromFile()
{
    QFile f("data/users.txt");
    if(!f.open(QIODevice::ReadOnly|QIODevice::Text)) return;
    QTextStream in(&f);
    while(!in.atEnd())
    {
        QString line = in.readLine();
        QStringList lst = line.split(" ");
        if(lst.size()>=2)
            users.append(User(lst[0],lst[1]));
    }
    f.close();
}

void FileSystem::saveFilesToFile()
{
    QFile f("data/files.txt");
    if(!f.open(QIODevice::WriteOnly|QIODevice::Text)) return;
    QTextStream out(&f);
    for(auto it=fileMap.begin();it!=fileMap.end();it++)
    {
        QString name = it.key();
        int id = it.value();
        out << name << "|" << inodeTable[id]->content << "\n";
    }
    f.close();
}

void FileSystem::loadFilesFromFile()
{
    QFile f("data/files.txt");
    if(!f.open(QIODevice::ReadOnly|QIODevice::Text)) return;
    QTextStream in(&f);
    while(!in.atEnd())
    {
        QString line = in.readLine();
        QStringList lst = line.split("|");
        if(lst.size()>=2)
        {
            createFile(lst[0]);
            writeFile(lst[0],lst[1]);
        }
    }
    f.close();
}
