#include "dbcreatediqlog.h"
#include "ui_dbcreatediqlog.h"
#include <QDebug>
#include <QMessageBox>

DBCreateDiqlog::DBCreateDiqlog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DBCreateDiqlog)
{
    ui->setupUi(this);
    //信号与槽连接
    connect(this->ui->cancelButton, SIGNAL(clicked(bool)), this, SLOT(cancle()));
    connect(this->ui->createButton, SIGNAL(clicked(bool)), this, SLOT(create()));
    dbService = new DBService();
}

DBCreateDiqlog::~DBCreateDiqlog()
{
    delete ui;
}

void DBCreateDiqlog::cancle(){
    this->close();
}

void DBCreateDiqlog::create(){
    QString databaseName = this->ui->lineEdit->text();
    if(databaseName.length()==0){
        QMessageBox::warning(nullptr,"警告","请输入数据库名");
    }else{
        if(dbService->createDatabase(databaseName)){
            emit sendData(this->ui->lineEdit->text());
            this->close();
        }else{
           QMessageBox::warning(nullptr,"警告","数据库名重复或不合法");
        }
    }
}

