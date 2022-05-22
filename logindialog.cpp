#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    ifFirstTimeUse();
    connect(ui->loginButton,SIGNAL(clicked(bool)),this,SLOT(loginUser()));
    connect(ui->registerButton,SIGNAL(clicked(bool)),this,SLOT(registerUser()));
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

vector<QString> LoginDialog::getUserList(){
    QString path = DB_CREATE_PATH;
    path.append("user.info");
    vector<QString> userList;
    if(FileUtil().ifExistFile(path)){
        int counter = FileUtil().getCounter(path);
        for(int i = 2;i <= counter+1;i++){
            QString userInfo = FileUtil().readLine(path,i);
            QStringList localSplit = userInfo.split(' ');
            userList.push_back(localSplit[0]);
        }
    }
    return userList;
}

vector<vector<QString>> LoginDialog::getUserListWithPassword(){
    QString path = DB_CREATE_PATH;
    path.append("user.info");
    vector<vector<QString>> userList;
    if(FileUtil().ifExistFile(path)){
        int counter = FileUtil().getCounter(path);
        for(int i = 2;i <= counter+1;i++){
            vector<QString> user;
            QString userInfo = FileUtil().readLine(path,i);
            QStringList localSplit = userInfo.split(' ');
            user.push_back(localSplit[0]);
            user.push_back(localSplit[1]);
            userList.push_back(user);
        }
    }
    return userList;
}

bool LoginDialog::ifFirstTimeUse(){
    QString path = DB_CREATE_PATH;
    path.append("system.db");
    if(!FileUtil().ifExistFile(path)){
        ui->accountEdit->setText("root");
        ui->accountEdit->setEnabled(false);
    }else{
        ui->label_3->hide();
        flag = 0;
    }
}

bool LoginDialog::registerUser(){
    QString account = ui->accountEdit->text();
    QString password = ui->passwordEdit->text();
    if(account.length()==0||password.length()==0){
        QMessageBox::warning(nullptr,"警告","用户名或密码不能为空");
        return false;
    }else{
        //判断是否注册的是root用户
        if(flag){
            QString path = "/Users/soulwinter/Desktop/DBMSData/";
            if(FileUtil().createFolder(path)){
                //创建system.dbwen文件和user.info文件
                QString path1 = DB_CREATE_PATH;
                path1.append("system.db");
                FileUtil().createFile(path1);
                FileUtil().writeAllFileForQString(path1,"0");
                
                QString path2 = DB_CREATE_PATH;
                path2.append("user.info");
                FileUtil().createFile(path2);
                FileUtil().writeAllFileForQString(path2,"1");
                QString content = account.append(" ").append(password);
                FileUtil().writeAtTail(path2,content);
                QMessageBox::about(nullptr,"成功","注册成功");
                return true;
            }else{
                QMessageBox::warning(nullptr,"警告","请确保文件夹未被占用");
                return false;
            }
        }else{
            //注册的普通用户
            //1判断用户名是否重复
            vector<QString> userList = getUserList();
            for(int i = 0 ;i < userList.size();i++){
                if(account.compare(userList.at(i))==0){
                    QMessageBox::warning(nullptr,"警告","用户名已存在");
                    return false;
                }
            }
            //2用户名不重复，进行注册
            QString path = DB_CREATE_PATH;
            path.append("user.info");
            QString content = account.append(" ").append(password);
            FileUtil().writeAtTail(path,content);
            FileUtil().updateCounter(path,QString::number(userList.size()+1));
            QMessageBox::about(nullptr,"成功","注册成功");
            return true;
        }
    }
    return false;
}

bool LoginDialog::loginUser(){
    QString account = ui->accountEdit->text();
    QString password = ui->passwordEdit->text();
    if(account.length()==0||password.length()==0){
        QMessageBox::warning(nullptr,"警告","用户名或密码不能为空");
        return false;
    }
    vector<vector<QString> > userListWithPassword = getUserListWithPassword();
    for(int i = 0;i < userListWithPassword.size();i++){
        if(account.compare(userListWithPassword.at(i).at(0))==0 && password.compare(userListWithPassword.at(i).at(1)) == 0){
            Dialog* dialog = new Dialog(account);
            dialog->show();
            this->close();
            return true;
        }
    }
    QMessageBox::warning(nullptr,"警告","不存在用户");
    return false;
}
