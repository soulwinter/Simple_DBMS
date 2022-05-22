#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "fileutil.h"
#include "global.h"
#include "dialog.h"
#include <vector>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();

private:
    Ui::LoginDialog *ui;
    bool ifFirstTimeUse();  //判断用户是否第一次使用我们的系统
    int flag = 1; //是否第一次使用
    vector<QString> getUserList();
    vector<vector<QString>> getUserListWithPassword();
private slots:
    bool registerUser();
    bool loginUser();
    
};

#endif // LOGINDIALOG_H
