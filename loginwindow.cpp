#include "loginwindow.h"
#include "ui_loginwindow.h"

#include <QMessageBox>
#include "mainwindow.h"
#include <QPushButton>

LoginWindow::LoginWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginWindow)
{
    ui->setupUi(this);

    connect(
        ui->loginBtn,
        &QPushButton::clicked,
        this,
        &LoginWindow::login
        );

    connect(
        ui->registerBtn,
        &QPushButton::clicked,
        this,
        &LoginWindow::registerUser
        );
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::login()
{
    QString username =
        ui->usernameEdit->text();

    QString password =
        ui->passwordEdit->text();

    if(username.isEmpty()
        || password.isEmpty())
    {
        QMessageBox::warning(
            this,
            "错误",
            "用户名和密码不能为空"
            );
        return;
    }

    if(filesystem.login(
            username,
            password))
    {
        MainWindow *w =
            new MainWindow(
                &filesystem);

        w->show();

        this->hide();
    }
    else
    {
        QMessageBox::warning(
            this,
            "登录失败",
            filesystem.lastError
            );
    }
}

void LoginWindow::registerUser()
{
    QString username =
        ui->usernameEdit->text();

    QString password =
        ui->passwordEdit->text();

    if(username.isEmpty()
        || password.isEmpty())
    {
        QMessageBox::warning(
            this,
            "错误",
            "用户名和密码不能为空"
            );
        return;
    }

    if(filesystem.registerUser(
            username,
            password))
    {
        QMessageBox::information(
            this,
            "提示",
            "注册成功"
            );
    }
    else
    {
        QMessageBox::warning(
            this,
            "注册失败",
            filesystem.lastError
            );
    }
}
