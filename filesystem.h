#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <QList>
#include <QMap>
#include <QString>
#include <QFile>
#include <QTextStream>

#include "user.h"
#include "inode.h"
#include "diskmanager.h"

class FileSystem
{
public:
    FileSystem();

    bool registerUser(QString username, QString password);
    bool login(QString username, QString password);

    bool createFile(QString filename);
    bool deleteFile(QString filename);
    bool writeFile(QString filename, QString content);
    QString readFile(QString filename);

    void saveUsersToFile();
    void loadUsersFromFile();
    void saveFilesToFile();
    void loadFilesFromFile();

private:
    QList<User> users;
    QMap<QString, int> fileMap;
    QMap<int, Inode*> inodeTable;
    DiskManager diskManager;
    QString currentUser;
};

#endif
