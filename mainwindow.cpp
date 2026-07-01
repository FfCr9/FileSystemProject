#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QTreeWidgetItem>

MainWindow::MainWindow(
    FileSystem *fs,
    QWidget *parent)
    :QMainWindow(parent)
    ,ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    filesystem = fs;

    // 隐藏树标题
    ui->fileTree->setHeaderHidden(true);

    connect(
        ui->createBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::createFile);

    connect(
        ui->deleteBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::deleteFile);

    connect(
        ui->openBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::openFile);

    connect(
        ui->saveBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::saveFile);

    connect(
        ui->mkdirBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::createDir);

    connect(
        ui->rmdirBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::removeDir);

    connect(
        ui->refreshBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::refreshList);

    connect(
        ui->searchBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::searchFile);

    // 点击左边文件树自动填入文件名
    connect(
        ui->fileTree,
        &QTreeWidget::itemClicked,
        this,
        [=](QTreeWidgetItem *item,int)
        {
            ui->fileNameEdit
                ->setText(
                    item->text(0));
        });

    refreshList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createFile()
{
    QString name =
        ui->fileNameEdit
            ->text();

    if(!filesystem->createFile(name))
    {
        QMessageBox::warning(
            this,
            "错误",
            filesystem->lastError);

        return;
    }

    refreshList();
}

void MainWindow::deleteFile()
{
    QString name =
        ui->fileNameEdit
            ->text();

    if(!filesystem->deleteFile(name))
    {
        QMessageBox::warning(
            this,
            "错误",
            filesystem->lastError);

        return;
    }

    refreshList();
}

void MainWindow::openFile()
{
    QString name =
        ui->fileNameEdit
            ->text();

    QString content =
        filesystem
            ->readFile(name);

    if(!filesystem
             ->lastError
             .isEmpty())
    {
        QMessageBox::warning(
            this,
            "错误",
            filesystem->lastError);

        return;
    }

    ui->contentEdit
        ->setPlainText(
            content);
}

void MainWindow::saveFile()
{
    QString name =
        ui->fileNameEdit
            ->text();

    QString content =
        ui->contentEdit
            ->toPlainText();

    if(!filesystem->writeFile(
            name,
            content))
    {
        QMessageBox::warning(
            this,
            "错误",
            filesystem->lastError);

        return;
    }

    QMessageBox::information(
        this,
        "提示",
        "保存成功");
}

void MainWindow::createDir()
{
    QString name =
        ui->fileNameEdit
            ->text();

    if(!filesystem
             ->createDirectory(name))
    {
        QMessageBox::warning(
            this,
            "错误",
            filesystem->lastError);

        return;
    }

    refreshList();
}

void MainWindow::removeDir()
{
    QString name =
        ui->fileNameEdit
            ->text();

    if(!filesystem
             ->rmdir(name))
    {
        QMessageBox::warning(
            this,
            "错误",
            filesystem->lastError);

        return;
    }

    refreshList();
}

void MainWindow::searchFile()
{
    QString name =
        ui->fileNameEdit
            ->text();

    QString path =
        filesystem
            ->searchFile(name);

    if(path.isEmpty())
    {
        QMessageBox::warning(
            this,
            "错误",
            filesystem->lastError);

        return;
    }

    QMessageBox::information(
        this,
        "搜索结果",
        path);
}

void MainWindow::refreshList()
{
    ui->fileTree->clear();

    QStringList list =
        filesystem
            ->listDirectory();

    for(QString s : list)
    {
        QTreeWidgetItem *item =
            new QTreeWidgetItem;

        item->setText(
            0,
            s);

        ui->fileTree
            ->addTopLevelItem(
                item);
    }
}
