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

bool FileSystem::appendFile(QString filename, QString content)
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
    node->content += content;
    node->size = node->content.length();
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

bool FileSystem::chmod(QString filename, QString mode)
{
    if(!fileMap.contains(filename))
    {
        lastError = "文件不存在";
        return false;
    }
    int bid = fileMap[filename];
    Inode* node = inodeTable[bid];
    if(node->owner != currentUser)
    {
        lastError = "仅文件所有者可修改权限";
        return false;
    }
    node->permission = mode;
    saveFilesToFile();
    lastError.clear();
    return true;
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

bool FileSystem::rmdir(QString dirname)
{
    for(int i=0;i<currentDir->childDirs.size();i++)
    {
        Directory* d = currentDir->childDirs[i];
        if(d->dirname == dirname)
        {
            if(!d->childDirs.isEmpty() || !d->files.isEmpty())
            {
                lastError = "目录不为空，无法删除";
                return false;
            }
            delete d;
            currentDir->childDirs.removeAt(i);
            lastError.clear();
            return true;
        }
    }
    lastError = "目录不存在";
    return false;
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

QStringList FileSystem::listDirectoryDetail()
{
    QStringList res;
    for(QString fname : currentDir->files)
    {
        int bid = fileMap[fname];
        Inode* node = inodeTable[bid];
        QString info = QString("%1 | 大小:%2 | 所有者:%3 | 权限:%4 | 创建:%5")
                .arg(fname)
                .arg(node->size)
                .arg(node->owner)
                .arg(node->permission)
                .arg(node->createTime.toString("yyyy-MM-dd HH:mm:ss"));
        res.append(info);
    }
    return res;
}

int FileSystem::openFile(QString filename)
{
    if(!fileMap.contains(filename))
    {
        lastError = "文件不存在";
        return -1;
    }
    int fd = fdCount++;
    openFileTable[fd] = filename;
    lastError.clear();
    return fd;
}

bool FileSystem::closeFile(int fd)
{
    if(!openFileTable.contains(fd))
    {
        lastError = "无效文件描述符";
        return false;
    }
    openFileTable.remove(fd);
    lastError.clear();
    return true;
}

QString FileSystem::recursiveSearch(Directory* dir, QString target, QString path)
{
    if(dir->files.contains(target))
    {
        return path + "/" + target;
    }
    for(Directory* sub : dir->childDirs)
    {
        QString res = recursiveSearch(sub, target, path + "/" + sub->dirname);
        if(!res.isEmpty()) return res;
    }
    return "";
}

QString FileSystem::searchFile(QString filename)
{
    QString fullPath = recursiveSearch(root, filename, "root");
    if(fullPath.isEmpty())
    {
        lastError = "未找到该文件";
        return "";
    }
    lastError.clear();
    return fullPath;
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
        Inode* node = inodeTable[id];
        QString timeStr = node->createTime.toString("yyyy-MM-dd HH:mm:ss");
        out << name << "|"
            << node->content << "|"
            << node->owner << "|"
            << node->permission << "|"
            << node->size << "|"
            << timeStr
            << "\n";
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
        if(lst.size() < 6) continue;
        QString fname = lst[0];
        QString cont = lst[1];
        QString own = lst[2];
        QString perm = lst[3];
        int sz = lst[4].toInt();
        QDateTime ctime = QDateTime::fromString(lst[5], "yyyy-MM-dd HH:mm:ss");

        createFile(fname);
        int bid = fileMap[fname];
        Inode* node = inodeTable[bid];
        node->content = cont;
        node->owner = own;
        node->permission = perm;
        node->size = sz;
        node->createTime = ctime;
    }
    f.close();
}
