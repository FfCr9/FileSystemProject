#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "filesystem.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(
        FileSystem *fs,
        QWidget *parent=nullptr);

    ~MainWindow();

private slots:

    void createFile();

    void deleteFile();

    void openFile();

    void saveFile();

    void createDir();

    void removeDir();

    void refreshList();

    void searchFile();

private:

    Ui::MainWindow *ui;

    FileSystem *filesystem;
};

#endif
