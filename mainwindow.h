#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "filesystem.h"
#include <QTreeWidgetItem>
#include <QFileDialog>

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

    void importTxtFile();

    void exportTxtFile();

    void enterDirectory(
        QTreeWidgetItem *item,
        int column);

private:

    Ui::MainWindow *ui;

    FileSystem *filesystem;
};

#endif
