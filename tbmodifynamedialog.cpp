#include "tbmodifynamedialog.h"
#include "ui_tbmodifynamedialog.h"

TBModifyNameDialog::TBModifyNameDialog(QString dbName,QString oldName,int type,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TBModifyNameDialog)
{
    ui->setupUi(this);
    this->type = type;
    this->oldName = oldName;
    this->dbName = dbName;
    //信号与槽连接
    connect(this->ui->cancelButton, SIGNAL(clicked(bool)), this, SLOT(cancle()));
    connect(this->ui->saveButton, SIGNAL(clicked(bool)), this, SLOT(save()));
}

TBModifyNameDialog::~TBModifyNameDialog()
{
    delete ui;
}


void TBModifyNameDialog::cancle(){
    this->close();
}

void TBModifyNameDialog::save(){
    QString newName = this->ui->lineEdit->text();
    //根据type修改对应的名称
    if(type==1){
        //1表示数据库
        if(DBService().modifyDatabaseName(oldName,newName)){
            QMessageBox::about(this,"成功","修改成功");

            //从user.info里修改数据库信息

            FileUtil().replaceContent("/Users/soulwinter/Desktop/DBMSData/user.info",oldName,newName);

            emit sendFlag();
            this->close();
        }else{
            QMessageBox::warning(this,"错误","数据库名已存在或不合法");
        }
    }else{
        TableService* tableService = new TableService();
        tableService->setDatabaseName(dbName);
        if(tableService->ModifyTableName(oldName,newName)==0){
            QMessageBox::about(this,"成功","修改成功");
            emit sendFlag();
            this->close();
        }else{
            QMessageBox::warning(this,"错误","表名已存在或不合法");
        }
    }
}
