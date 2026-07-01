#include "loginwindow.h"
#include "ui_loginwindow.h"

#include "mainwindow.h"

#include <QMessageBox>
#include <QPushButton>

extern FileSystem g_fs;

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

    bool ret = g_fs.login(username, password);
       if(!ret)
       {
           QMessageBox::warning(this,"登录失败",g_fs.lastError);
           return;
       }

    MainWindow *w =
        new MainWindow;

    w->show();

    close();
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

    bool ret = g_fs.registerUser(username,password);
      if(!ret)
      {
          QMessageBox::warning(this,"注册失败",g_fs.lastError);
          return;
      }

    QMessageBox::information(
        this,
        "成功",
        "注册成功(模拟)"
        );
}
