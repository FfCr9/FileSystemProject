#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QPushButton>

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
    QMessageBox::information(
        this,
        "提示",
        "创建文件"
        );
}

void MainWindow::deleteFile()
{
    QMessageBox::information(
        this,
        "提示",
        "删除文件"
        );
}

void MainWindow::createDir()
{
    QMessageBox::information(
        this,
        "提示",
        "创建目录"
        );
}

void MainWindow::saveFile()
{
    QMessageBox::information(
        this,
        "提示",
        "保存文件"
        );
}

void MainWindow::searchFile()
{
    QMessageBox::information(
        this,
        "提示",
        "搜索文件"
        );
}
s
