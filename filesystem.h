#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QList>
#include <QMap>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QStringList>

#include "user.h"
#include "inode.h"
#include "diskmanager.h"
#include "directory.h"

class FileSystem
{
public:
    FileSystem();
    ~FileSystem();

    bool registerUser(QString username, QString password);
    bool login(QString username, QString password);
    void setCurrentUser(QString name);

    bool createFile(QString filename);
    bool deleteFile(QString filename);
    bool writeFile(QString filename, QString content);
    QString readFile(QString filename);
    bool appendFile(QString filename, QString content);

    bool chmod(QString filename, QString mode);
    bool createDirectory(QString dirname);
    bool rmdir(QString dirname);
    bool cd(QString dirname);
    QStringList listDirectory();
    QStringList listDirectoryDetail();

    int openFile(QString filename);
    bool closeFile(int fd);
    QString searchFile(QString filename);

    bool hasPermission(Inode* node, QString mode);

    void saveUsersToFile();
    void loadUsersFromFile();
    void saveFilesToFile();
    void loadFilesFromFile();

    QString lastError;

private:
    QString recursiveSearch(Directory* dir, QString target, QString path);

    QList<User> users;
    QMap<QString, int> fileMap;
    QMap<int, Inode*> inodeTable;
    DiskManager diskManager;
    QString currentUser;

    Directory* root;
    Directory* currentDir;

    QMap<int, QString> openFileTable;
    int fdCount;
};

#endif
