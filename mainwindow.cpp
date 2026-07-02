#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QTreeWidgetItem>
#include <QFileDialog>

MainWindow::MainWindow(
    FileSystem *fs,
    QWidget *parent)
    :QMainWindow(parent)
    ,ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    filesystem=fs;

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

    connect(
        ui->importBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::importTxtFile);

    connect(
        ui->exportBtn,
        &QPushButton::clicked,
        this,
        &MainWindow::exportTxtFile);

    connect(
        ui->backBtn,
        &QPushButton::clicked,
        this,
        [=]()
        {
            if(!filesystem->cd(".."))
            {
                QMessageBox::warning(
                    this,
                    "错误",
                    filesystem->lastError);

                return;
            }

            refreshList();
            ui->contentEdit->clear();
            ui->fileNameEdit->clear();
        });

    connect(
        ui->fileTree,
        &QTreeWidget::itemClicked,
        this,
        [=](QTreeWidgetItem *item,int)
        {
            QString text=
                item->text(0);

            text.remove("[DIR] ");

            ui->fileNameEdit
                ->setText(text);
        });
    connect(
        ui->fileTree,
        &QTreeWidget::itemDoubleClicked,
        this,
        &MainWindow::enterDirectory);

    refreshList();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::createFile()
{
    QString name=
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
    QString name=
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
    QString name=
        ui->fileNameEdit
            ->text();

    QString content=
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
    QString name=
        ui->fileNameEdit
            ->text();

    QString content=
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
    QString name=
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
    QString name=
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
    QString name=
        ui->fileNameEdit
            ->text();

    QString path=
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

    QStringList list=
        filesystem
            ->listDirectory();

    for(QString s:list)
    {
        QTreeWidgetItem *item=
            new QTreeWidgetItem;

        item->setText(
            0,
            s);

        ui->fileTree
            ->addTopLevelItem(
                item);
    }
}

void MainWindow::importTxtFile()
{
    QString path=
        QFileDialog::
        getOpenFileName(
            this,
            "选择txt文件",
            "",
            "*.txt");

    if(path.isEmpty())
        return;

    QString filename=
        ui->fileNameEdit
            ->text();

    if(filename.isEmpty())
    {
        QFileInfo fi(path);
        filename = fi.fileName();
    }

    if(!filesystem
             ->importTxt(
                 filename,
                 path))
    {
        QMessageBox::warning(
            this,
            "错误",
            filesystem->lastError);

        return;
    }

    refreshList();
    ui->fileNameEdit->clear();
}

void MainWindow::exportTxtFile()
{
    QString path=
        QFileDialog::
        getSaveFileName(
            this,
            "保存txt文件",
            "",
            "*.txt");

    if(path.isEmpty())
        return;

    QString filename=
        ui->fileNameEdit
            ->text();

    if(!filesystem
             ->exportTxt(
                 filename,
                 path))
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
        "导出成功");
}

void MainWindow::enterDirectory(
    QTreeWidgetItem *item,
    int)
{
    QString text=
        item->text(0);

    // 不是目录
    if(!text.startsWith("[DIR]"))
        return;

    QString dirname=
        text.mid(6);

    if(!filesystem->cd(dirname))
    {
        QMessageBox::warning(
            this,
            "错误",
            filesystem->lastError);

        return;
    }

    refreshList();
}
