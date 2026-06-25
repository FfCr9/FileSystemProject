#include "filesystem.h"

FileSystem::FileSystem()
{
    root = new Directory("root");
    currentDir = root;
    lastError.clear();

    loadUsersFromFile();
    loadFilesFromFile();
}

FileSystem::~FileSystem()
{
    for(auto &p : inodeTable)
    {
        delete p;
    }
    delete root;
}

void FileSystem::setCurrentUser(QString name)
{
    currentUser = name;
}

bool FileSystem::hasPermission(Inode *node, QString mode)
{
    if(node->owner != currentUser)
        return false;
    return node->permission.contains(mode);
}

bool FileSystem::registerUser(QString username, QString password)
{
    for(auto &u : users)
    {
        if(u.username == username)
        {
            lastError = "账号已存在";
            return false;
        }
    }
    users.append(User(username, password));
    saveUsersToFile();
    lastError.clear();
    return true;
}

bool FileSystem::login(QString username, QString password)
{
    for(auto &u : users)
    {
        if(u.username == username && u.password == password)
        {
            currentUser = username;
            lastError.clear();
            return true;
        }
    }
    lastError = "账号或密码错误";
    return false;
}

bool FileSystem::createFile(QString filename)
{
    if(currentDir->files.contains(filename))
    {
        lastError = "当前目录文件已存在";
        return false;
    }
    if(fileMap.contains(filename))
    {
        lastError = "文件已存在";
        return false;
    }

    int bid = diskManager.allocateBlock();
    if(bid == -1)
    {
        lastError = "磁盘空间已满";
        return false;
    }

    Inode *node = new Inode;
    node->inodeID = bid;
    node->owner = currentUser;
    node->permission = "rw";
    node->size = 0;
    node->content = "";

    inodeTable[bid] = node;
    fileMap[filename] = bid;
    currentDir->files.append(filename);

    saveFilesToFile();
    lastError.clear();
    return true;
}

bool FileSystem::deleteFile(QString filename)
{
    if(!fileMap.contains(filename))
    {
        lastError = "文件不存在";
        return false;
    }
    int bid = fileMap[filename];
    Inode* node = inodeTable[bid];

    if(!hasPermission(node,"w"))
    {
        lastError = "无权限删除";
        return false;
    }

    diskManager.freeBlock(bid);
    delete inodeTable[bid];
    inodeTable.remove(bid);
    fileMap.remove(filename);
    currentDir->files.removeOne(filename);

    saveFilesToFile();
    lastError.clear();
    return true;
}

bool FileSystem::writeFile(QString filename, QString content)
{
    if(!fileMap.contains(filename))
    {
        lastError = "文件不存在";
        return false;
    }
    int bid = fileMap[filename];
    Inode* node = inodeTable[bid];

    if(!hasPermission(node,"w"))
    {
        lastError = "无写入权限";
        return false;
    }

    node->content = content;
    node->size = content.length();
    saveFilesToFile();
    lastError.clear();
    return true;
}

QString FileSystem::readFile(QString filename)
{
    if(!fileMap.contains(filename))
    {
        lastError = "文件不存在";
        return "";
    }
    int bid = fileMap[filename];
    Inode* node = inodeTable[bid];

    if(!hasPermission(node,"r"))
    {
        lastError = "无读取权限";
        return "";
    }
    lastError.clear();
    return inodeTable[bid]->content;
}

bool FileSystem::createDirectory(QString dirname)
{
    for(Directory* d : currentDir->childDirs)
    {
        if(d->dirname == dirname)
        {
            lastError = "目录已存在";
            return false;
        }
    }
    Directory* newDir = new Directory(dirname);
    newDir->parent = currentDir;
    currentDir->childDirs.append(newDir);
    lastError.clear();
    return true;
}

bool FileSystem::cd(QString dirname)
{
    if(dirname == "..")
    {
        if(currentDir->parent != nullptr)
        {
            currentDir = currentDir->parent;
            lastError.clear();
            return true;
        }
        lastError = "已是根目录，无法退回";
        return false;
    }
    for(Directory* d : currentDir->childDirs)
    {
        if(d->dirname == dirname)
        {
            currentDir = d;
            lastError.clear();
            return true;
        }
    }
    lastError = "目录不存在";
    return false;
}

QStringList FileSystem::listDirectory()
{
    QStringList res;
    for(QString f : currentDir->files)
    {
        res.append(f);
    }
    return res;
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
