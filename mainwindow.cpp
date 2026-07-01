#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QTreeWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    :QMainWindow(parent)
    ,ui(new Ui::MainWindow)
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

    ui->treeWidget->clear();

    QTreeWidgetItem *root =
        new QTreeWidgetItem;

    root->setText(
        0,
        "root"
        );

    ui->treeWidget
        ->addTopLevelItem(root);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showError()
{
    QString msg=
        filesystem.lastError;

    if(msg.isEmpty())
    {
        msg="未知错误";
    }

    QMessageBox::warning(
        this,
        "操作失败",
        msg
        );
}

void MainWindow::createFile()
{
    QString filename=
        QInputDialog::getText(
            this,
            "创建文件",
            "输入文件名："
            );

    if(filename.isEmpty())
        return;

    bool ok=
        filesystem.createFile(
            filename
            );

    if(!ok)
    {
        showError();
        return;
    }

    QTreeWidgetItem *item=
        new QTreeWidgetItem;

    item->setText(
        0,
        filename
        );

    ui->treeWidget
        ->topLevelItem(0)
        ->addChild(item);
}

void MainWindow::deleteFile()
{
    auto item=
        ui->treeWidget
            ->currentItem();

    if(item==nullptr)
    {
        QMessageBox::warning(
            this,
            "错误",
            "请选择文件"
            );

        return;
    }

    QString filename=
        item->text(0);

    bool ok=
        filesystem.deleteFile(
            filename
            );

    if(!ok)
    {
        showError();
        return;
    }

    delete item;
}

void MainWindow::createDir()
{
    QString dirname=
        QInputDialog::getText(
            this,
            "创建目录",
            "输入目录名："
            );

    if(dirname.isEmpty())
        return;

    bool ok=
        filesystem.createDirectory(
            dirname
            );

    if(!ok)
    {
        showError();
        return;
    }

    QTreeWidgetItem *item=
        new QTreeWidgetItem;

    item->setText(
        0,
        "[DIR] "+dirname
        );

    ui->treeWidget
        ->topLevelItem(0)
        ->addChild(item);
}

void MainWindow::saveFile()
{
    QString text=
        ui->textEdit->toPlainText();

    auto item=
        ui->treeWidget
            ->currentItem();

    if(item==nullptr)
    {
        QMessageBox::warning(
            this,
            "错误",
            "未选择文件"
            );

        return;
    }

    QString filename=
        item->text(0);

    bool ok=
        filesystem.writeFile(
            filename,
            text
            );

    if(!ok)
    {
        showError();
        return;
    }

    QMessageBox::information(
        this,
        "成功",
        "保存成功"
        );
}

void MainWindow::searchFile()
{
    QString filename=
        QInputDialog::getText(
            this,
            "搜索",
            "输入文件名"
            );

    if(filename.isEmpty())
        return;

    QString path=
        filesystem.searchFile(
            filename
            );

    if(path.isEmpty())
    {
        showError();
        return;
    }

    QMessageBox::information(
        this,
        "搜索结果",
        path
        );
}
