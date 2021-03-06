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
        QMessageBox::warning(nullptr,"??????","??????????????????????????????");
        return false;
    }else{
        //????????????????????????root??????
        if(flag){
            QString path = "/Users/soulwinter/Desktop/DBMSData/";
            if(FileUtil().createFolder(path)){
                //??????system.dbwen?????????user.info??????
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
                QMessageBox::about(nullptr,"??????","????????????");
                return true;
            }else{
                QMessageBox::warning(nullptr,"??????","??????????????????????????????");
                return false;
            }
        }else{
            //?????????????????????
            //1???????????????????????????
            vector<QString> userList = getUserList();
            for(int i = 0 ;i < userList.size();i++){
                if(account.compare(userList.at(i))==0){
                    QMessageBox::warning(nullptr,"??????","??????????????????");
                    return false;
                }
            }
            //2?????????????????????????????????
            QString path = DB_CREATE_PATH;
            path.append("user.info");
            QString content = account.append(" ").append(password);
            FileUtil().writeAtTail(path,content);
            FileUtil().updateCounter(path,QString::number(userList.size()+1));
            QMessageBox::about(nullptr,"??????","????????????");
            return true;
        }
    }
    return false;
}

bool LoginDialog::loginUser(){
    QString account = ui->accountEdit->text();
    QString password = ui->passwordEdit->text();
    if(account.length()==0||password.length()==0){
        QMessageBox::warning(nullptr,"??????","??????????????????????????????");
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
    QMessageBox::warning(nullptr,"??????","???????????????");
    return false;
}
