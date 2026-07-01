#include "loginwindow.h"
#include <QApplication>
#include "filesystem.h"   // 新增

FileSystem g_fs;           // 新增全局对象

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    LoginWindow w;
    w.show();
    return a.exec();
}
