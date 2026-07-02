#include "filesystem.h"
#include <QDir>
#include <QCoreApplication>

FileSystem::FileSystem()
{
    root = new Directory("root");
    currentDir = root;
    lastError.clear();
    fdCount = 0;

    QString dataPath =
        QCoreApplication::applicationDirPath()
        + "/data";

    QDir dir;

    if(!dir.exists(dataPath))
    {
        dir.mkpath(dataPath);
    }

    loadUsersFromFile();
    loadFilesFromFile();
}

FileSystem::~FileSystem()
{
    saveUsersToFile();
    saveFilesToFile();

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
    if(node->owner == currentUser)
        return true;
    return node->permission.contains(mode);
}

bool FileSystem::removeFileFromDir(Directory* dir, QString filename)
{
    if(dir->files.contains(filename))
    {
        dir->files.removeOne(filename);
        return true;
    }
    for(Directory* sub : dir->childDirs)
    {
        if(removeFileFromDir(sub, filename))
            return true;
    }
    return false;
}

bool FileSystem::registerUser(
    QString username,
    QString password)
{
    username=username.trimmed();
    password=password.trimmed();

    if(username.isEmpty()
        || password.isEmpty())
    {
        lastError="用户名密码不能为空";
        return false;
    }

    for(auto &u:users)
    {
        if(u.username==username)
        {
            lastError="账号已存在";
            return false;
        }
    }

    users.append(
        User(
            username,
            password));

    saveUsersToFile();

    lastError.clear();

    return true;
}

bool FileSystem::login(
    QString username,
    QString password)
{
    username=username.trimmed();
    password=password.trimmed();

    for(auto &u:users)
    {
        if(u.username==username
            &&
            u.password==password)
        {
            currentUser=username;

            lastError.clear();

            return true;
        }
    }

    lastError="账号或密码错误";

    return false;
}

bool FileSystem::createFile(QString filename)
{
    for(Directory* d : currentDir->childDirs)
    {
        if(d->dirname == filename)
        {
            lastError = "同名目录已存在";
            return false;
        }
    }

    if(currentDir->files.contains(filename))
    {
        lastError = "目录文件名已存在";
        return false;
    }
    if(fileMap.contains(filename))
    {
        lastError = "文件名已存在";
        return false;
    }

    int bid = diskManager.allocateBlock();
    if(bid == -1)
    {
        lastError = "磁盘空间不足";
        return false;
    }

    Inode *node = new Inode;
    node->inodeID = bid;
    node->owner = currentUser;
    node->permission = "rw";
    node->size = 0;
    node->content = "";
    node->createTime = QDateTime::currentDateTime();

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
        lastError = "文件名不存在";
        return false;
    }
    int bid = fileMap[filename];
    Inode* node = inodeTable[bid];

    if(!hasPermission(node,"w"))
    {
        lastError = "权限不足";
        return false;
    }

    diskManager.freeBlock(bid);
    delete inodeTable[bid];
    inodeTable.remove(bid);
    fileMap.remove(filename);
    removeFileFromDir(root, filename);

    saveFilesToFile();
    lastError.clear();
    return true;
}

bool FileSystem::writeFile(QString filename, QString content)
{
    if(!fileMap.contains(filename))
    {
        lastError = "文件名不存在";
        return false;
    }
    int bid = fileMap[filename];
    Inode* node = inodeTable[bid];

    if(!hasPermission(node,"w"))
    {
        lastError = "权限不足";
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
        lastError = "文件名不存在";
        return false;
    }
    int bid = fileMap[filename];
    Inode* node = inodeTable[bid];
    if(!hasPermission(node,"w"))
    {
        lastError = "权限不足";
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
        lastError = "文件名不存在";
        return "";
    }
    int bid = fileMap[filename];
    Inode* node = inodeTable[bid];

    if(!hasPermission(node,"r"))
    {
        lastError = "权限不足";
        return "";
    }
    lastError.clear();
    return inodeTable[bid]->content;
}

QString FileSystem:: getCurrentPath()
{
    QString path;
    Directory *dir= currentDir;
    while(dir)
    {
        path= "/" + dir->dirname+ path;
        dir= dir->parent;
    }
    return path;
}

bool FileSystem::chmod(QString filename, QString mode)
{
    if(!fileMap.contains(filename))
    {
        lastError = "文件名不存在";
        return false;
    }
    int bid = fileMap[filename];
    Inode* node = inodeTable[bid];
    if(node->owner != currentUser)
    {
        lastError = "权限不足，仅所有者可修改权限";
        return false;
    }
    node->permission = mode;
    saveFilesToFile();
    lastError.clear();
    return true;
}

bool FileSystem::createDirectory(QString dirname)
{
    if(currentDir->files.contains(dirname))
    {
        lastError = "同名文件已存在";
        return false;
    }

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

    saveFilesToFile();
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
            saveFilesToFile();
            lastError.clear();
            return true;
        }
    }
    lastError = "目录不存在";
    return false;
}

bool FileSystem::cd( QString path)
{
    Directory *target;
    if(path.startsWith("root"))
        target=root;
    else
        target=currentDir;
    QStringList dirs= path.split( "/", Qt::SkipEmptyParts);
    for(QString dir:dirs)
    {
        if(dir=="..")
        {
            if(target->parent)
                target= target->parent;
            continue;
        }
        bool found=false;
        for(Directory *d: target->childDirs)
        {
            if(d->dirname==dir)
            {
                target=d;
                found=true;
                break;
            }
        }
        if(!found)
        {
            lastError= "路径不存在";
            return false;
        }
    }
    currentDir=target;
    lastError.clear();
    return true;
}

QStringList FileSystem::listDirectory()
{
    QStringList res;
    for(Directory *d : currentDir->childDirs)
    {
        res.append("[DIR] " + d->dirname);
    }
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
        lastError = "文件名不存在";
        return -1;
    }
    int bid = fileMap[filename];
    Inode *node = inodeTable[bid];
    if(!hasPermission(node,"r"))
    {
        lastError = "权限不足";
        return -1;
    }

    for(auto it = openFileTable.begin();
        it != openFileTable.end();
        ++it)
    {
        if(it.value() == filename)
        {
            lastError = "文件已打开";
            return it.key();
        }
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
        lastError = "文件不存在";
        return "";
    }
    lastError.clear();
    return fullPath;
}

void FileSystem::saveUsersToFile()
{
    QString filePath =
        QCoreApplication::applicationDirPath()
        + "/data/users.txt";

    QFile f(filePath);

    if(!f.open(
            QIODevice::WriteOnly
            |QIODevice::Text
            |QIODevice::Truncate))
    {
        lastError="用户保存失败";
        return;
    }

    QTextStream out(&f);

    for(auto &u:users)
    {
        out
            <<u.username
            <<" "
            <<u.password
            <<"\n";
    }

    f.close();
}

void FileSystem::loadUsersFromFile()
{
    users.clear();

    QString filePath =
        QCoreApplication::applicationDirPath()
        + "/data/users.txt";

    QFile f(filePath);

    if(!f.open(
            QIODevice::ReadOnly
            |QIODevice::Text))
    {
        return;
    }

    QTextStream in(&f);

    while(!in.atEnd())
    {
        QString line =
            in.readLine()
                .trimmed();

        if(line.isEmpty())
            continue;

        QStringList lst=
            line.split(
                " ",
                Qt::SkipEmptyParts);

        if(lst.size()<2)
            continue;

        users.append(
            User(
                lst[0],
                lst[1]));
    }

    f.close();
}

void FileSystem::saveDirectories(
        Directory *dir,
        QTextStream &out,
        QString path)
{
    for(Directory *sub : dir->childDirs)
    {
        if(sub == nullptr)
            continue;
        QString full = path + "/" + sub->dirname;

        out << "DIR|"
            << full
            << "\n";

        saveDirectories(
                sub,
                out,
                full);
    }
}

void FileSystem::saveFilesInDirectory(
        Directory *dir,
        QTextStream &out,
        QString path)
{
    for(QString fileName : dir->files)
    {
        if(!fileMap.contains(fileName))
            continue;

        int id = fileMap[fileName];
        Inode *node = inodeTable[id];

        QString timeStr =
                node->createTime.toString(
                    "yyyy-MM-dd HH:mm:ss");

        out << path << "/"
            << fileName << "|"
            << encryptText(node->content) << "|"
            << node->owner << "|"
            << node->permission << "|"
            << node->size << "|"
            << timeStr
            << "\n";
    }

    for(Directory *sub : dir->childDirs)
    {
        if(sub == nullptr)
            continue;

        saveFilesInDirectory(
                sub,
                out,
                path + "/" + sub->dirname);
    }
}

void FileSystem::loadDirectoryByPath(QString path)
{
    QStringList dirs = path.split("/");

    Directory *current = root;

    for(int i = 1; i < dirs.size(); i++)
    {
        QString dirname = dirs[i];

        Directory *next = nullptr;

        for(Directory *sub : current->childDirs)
        {
            if(sub->dirname == dirname)
            {
                next = sub;
                break;
            }
        }

        if(next == nullptr)
        {
            next = new Directory(dirname);
            next->parent = current;
            current->childDirs.append(next);
        }

        current = next;
    }
}

void FileSystem::saveFilesToFile()
{
    QString filePath=
        QCoreApplication::applicationDirPath()
        +"/data/files.txt";

    QFile f(filePath);

    if(!f.open(
            QIODevice::WriteOnly
            |QIODevice::Text
            |QIODevice::Truncate))
    {
        lastError="文件数据保存失败";
        return;
    }

    QTextStream out(&f);

    saveDirectories(
        root,
        out,
        "root");

    saveFilesInDirectory(
        root,
        out,
        "root");

    f.close();
}

void FileSystem::loadFilesFromFile()
{
    QString filePath=
        QCoreApplication::applicationDirPath()
        +"/data/files.txt";

    QFile f(filePath);
    if(!f.open(QIODevice::ReadOnly|QIODevice::Text)) return;
    QTextStream in(&f);
    while(!in.atEnd())
    {
        QString line = in.readLine();
        QStringList lst = line.split("|");
        if(lst.isEmpty())
            continue;
        if(lst[0] == "DIR")
        {
            if(lst.size() >= 2)
                loadDirectoryByPath(lst[1]);

            continue;
        }


        if(lst.size() < 6) continue;
        QString fullPath = lst[0];

        QStringList parts = fullPath.split("/");

        QString fname = parts.last();
        QString cont = decryptText(lst[1]);
        QString own = lst[2];
        QString perm = lst[3];

        int sz = lst[4].toInt();

        QDateTime ctime =
                QDateTime::fromString(
                    lst[5],
                    "yyyy-MM-dd HH:mm:ss");

        int bid = diskManager.allocateBlock();

        if(bid == -1)
            continue;

        Inode *node = new Inode;

        node->inodeID = bid;
        node->owner = own;
        node->permission = perm;
        node->size = sz;
        node->content = cont;
        node->createTime = ctime;

        inodeTable[bid] = node;

        fileMap[fname] = bid;

        Directory *target = root;

        for(int i = 1; i < parts.size() - 1; i++)
        {
            QString dirName = parts[i];

            for(Directory *d : target->childDirs)
            {
                if(d->dirname == dirName)
                {
                    target = d;
                    break;
                }
            }
        }

        target->files.append(fname);
    }
    f.close();
}


QString FileSystem::readByFd( int fd)
{
    if(!openFileTable.contains(fd))
    {
        lastError="无效文件描述符";
        return "";
    }
    QString filename= openFileTable[fd];
    return readFile( filename);
}

bool FileSystem::writeByFd( int fd, QString content)
{
    if(!openFileTable.contains(fd))
    {
        lastError="无效文件描述符";
        return false;
    }
    QString filename= openFileTable[fd];
    return writeFile( filename, content);
}


bool FileSystem::importTxt(QString sysFileName, QString localTxtPath)
{
    QFile localFile(localTxtPath);
    if (!localFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        lastError = "本地文件打开失败，路径不存在或权限不足";
        return false;
    }
    QTextStream stream(&localFile);
    QString content = stream.readAll();
    localFile.close();

    if (!fileMap.contains(sysFileName))
    {
        if (!createFile(sysFileName))
            return false;
    }
    bool res = writeFile(sysFileName, content);
    if (res) lastError.clear();
    return res;
}

bool FileSystem::exportTxt(QString sysFileName, QString localTxtPath)
{
    if (!fileMap.contains(sysFileName))
    {
        lastError = "文件名不存在";
        return false;
    }
    QString content = readFile(sysFileName);
    if (!lastError.isEmpty())
        return false;

    QFile outFile(localTxtPath);
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        lastError = "权限不足";
        return false;
    }
    QTextStream stream(&outFile);
    stream << content;
    outFile.close();
    lastError.clear();
    return true;
}

QString FileSystem::encryptText(const QString &text)
{
    QString result;
    for (int i = 0; i < text.size(); i++)
    {
        QChar ch = text[i];
        QChar keyCh = encryptKey[i % encryptKey.size()];
        result += QChar(ch.unicode() ^ keyCh.unicode());
    }
    return result;
}

QString FileSystem::decryptText(const QString &text)
{
    return encryptText(text);
}
