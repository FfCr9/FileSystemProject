#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QPushButton>
#include <QInputDialog>

extern FileSystem g_fs;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(
        ui->createBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::createFile
        );

    connect(
        ui->deleteBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::deleteFile
        );

    connect(
        ui->mkdirBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::createDir
        );

    connect(
        ui->saveBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::saveFile
        );

    connect(
        ui->searchBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::searchFile
        );
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createFile()
{
    QString name = QInputDialog::getText(this,"新建文件","输入文件名");
    if(name.isEmpty())return;
    bool ok = g_fs.createFile(name);
    if(!ok)
    {
        QMessageBox::warning(this,"操作失败",g_fs.lastError);
    }
    else
    {
        QMessageBox::information(this,"提示","创建文件成功");
    }
}

void MainWindow::deleteFile()
{
    QString name = QInputDialog::getText(this,"删除文件","输入文件名");
    if(name.isEmpty())return;
    bool ok = g_fs.deleteFile(name);
    if(!ok)
    {
        QMessageBox::warning(this,"操作失败",g_fs.lastError);
    }
    else
    {
        QMessageBox::information(this,"提示","删除文件成功");
    }
}

void MainWindow::createDir()
{
    QString name = QInputDialog::getText(this,"新建目录","输入目录名");
    if(name.isEmpty())return;
    bool ok = g_fs.createDirectory(name);
    if(!ok)
    {
        QMessageBox::warning(this,"操作失败",g_fs.lastError);
    }
    else
    {
        QMessageBox::information(this,"提示","创建目录成功");
    }
}

void MainWindow::saveFile()
{
    QString name = QInputDialog::getText(this,"写入文件","输入文件名");
    if(name.isEmpty())return;
    QString content = ui->textEdit->toPlainText();
    bool ok = g_fs.writeFile(name,content);
    if(!ok)
    {
        QMessageBox::warning(this,"操作失败",g_fs.lastError);
    }
    else
    {
        QMessageBox::information(this,"提示","保存文件成功");
    }
}

void MainWindow::searchFile()
{
    QString name = QInputDialog::getText(this,"读取文件","输入文件名");
    if(name.isEmpty())return;
    QString txt = g_fs.readFile(name);
    if(!g_fs.lastError.isEmpty())
    {
        QMessageBox::warning(this,"读取失败",g_fs.lastError);
        return;
    }
    ui->textEdit->setPlainText(txt);
}
