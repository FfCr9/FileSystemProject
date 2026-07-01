#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QDialog>
#include "filesystem.h"

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void login();
    void registerUser();

private:
    Ui::LoginWindow *ui;

    FileSystem filesystem;
};

#endif // LOGINWINDOW_H
