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
    void logout();

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

    bool importTxt(QString sysFileName, QString localTxtPath);
    bool exportTxt(QString sysFileName, QString localTxtPath);

    void saveUsersToFile();
    void loadUsersFromFile();
    void saveFilesToFile();
    void loadFilesFromFile();

    QString lastError;

    QString readByFd(int fd);
    bool writeByFd( int fd, QString content);

    QString getCurrentPath();

private:
    QString recursiveSearch(Directory* dir, QString target, QString path);

    bool removeFileFromDir(Directory* dir, QString filename);
    bool isValidPathName(const QString& name);
    void setLastError(const QString& err);

    QList<User> users;
    QMap<QString, int> fileMap;
    QMap<int, Inode*> inodeTable;
    DiskManager diskManager;
    QString currentUser;

    Directory* root;
    Directory* currentDir;

    void saveDirectories(
            Directory* dir,
            QTextStream &out,
            QString path);

    void saveFilesInDirectory(Directory *dir,
                              QTextStream &out,
                              QString path);

    void loadDirectoryByPath(
            QString path);

    //优化：文件加密存储
    QString encryptText(const QString &text);
    QString decryptText(const QString &text);
    const QString encryptKey = "OSFileSys2026";

    QMap<int, QString> openFileTable;
    int fdCount;

    QString m_lastError;
};

#endif
