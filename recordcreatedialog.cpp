#include "recordcreatedialog.h"
#include "ui_recordcreatedialog.h"
#include "QMessageBox"

RecordCreateDialog::RecordCreateDialog(RecordEntity recordEntity,QString databaseName,QString tableName,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RecordCreateDialog)
{
    ui->setupUi(this);
    this->databaseName = databaseName;
    this->tableName = tableName;
    this->recordEntity = recordEntity;
    connect(ui->saveButton,SIGNAL(clicked(bool)),this,SLOT(save()));
    connect(ui->cancelButton,SIGNAL(clicked(bool)),this,SLOT(cancel()));
    init();
}

RecordCreateDialog::~RecordCreateDialog()
{
    delete ui;
}

//保存记录
void RecordCreateDialog::save(){

    //初始化RecordEntity实体
    RecordEntity record;
    int length = ui->tableWidget->rowCount();
    for(int i = 0;i<length;i++){
        QString fieldName = ui->tableWidget->item(i,0)->text();
        QString value = ui->tableWidget->item(i,5)->text();
        if(value.length()==0){
            record.SetValue(fieldName,"null");
        }else{
            record.SetValue(fieldName,value);
        }

    }

    //调用服务层接口保存记录
    //首先判断是新增还是修改
    int result = 0;
    if(recordEntity.getId()==-1){
        result = RecordService().insertRecord(databaseName,tableName,record);
    }else{
        result = RecordService().modifyrecord(databaseName,tableName,recordEntity,record);
    }
    if(result!=1){
        if(result == 8009){
            QMessageBox::warning(this,"错误","字段不允许为空");
        }
        if(result == 8008){
            QMessageBox::warning(this,"错误","长度溢出");
        }
        if(result == 8007){
            QMessageBox::warning(this,"错误","超过了约束范围");
        }
        if(result == 8005){
            QMessageBox::warning(this,"错误","主键或唯一重复");
        }
        if(result == 8010){
            QMessageBox::warning(this,"错误","不存在字段");
        }
        if(result == 10086){
            QMessageBox::warning(this,"错误","字段类型格式不正确或不匹配");
        }
        if(result == 10087){
            QMessageBox::warning(this,"错误","修改字段类型错误：null");
        }
        if(result == 10088){
            QMessageBox::warning(this,"错误","修改字段类型错误：length");
        }
        if(result == 10089){
            QMessageBox::warning(this,"错误","修改字段类型错误：max or min");
        }
        if(result == 10090){
            QMessageBox::warning(this,"错误","修改字段类型错误: PK or unique");
        }
    }else{
        QMessageBox::about(this,"成功","修改或添加记录成功");
        this->close();
    }

}

void RecordCreateDialog::cancel(){
    this->close();
}

//初始化方法
void RecordCreateDialog::init(){
    vector<FieldEntity> fieldList = FieldService().getFieldList(databaseName,tableName);
    //将字段显示在视图上
    for(int i=0;i<fieldList.size();i++){
        int row = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount(row+1);
        FieldEntity field = fieldList.at(i);
        //名字
        ui->tableWidget->setItem(row,0,new QTableWidgetItem(field.getName()));
        ui->tableWidget->item(row,0)->setFlags(Qt::ItemIsEnabled);
        //类型
        int type = field.getType();
        QString typeStr = "";
        switch (type) {
        case 1:
            typeStr = "int";
            break;
        case 2:
            typeStr = "bool";
            break;
        case 3:
            typeStr = "time";
            break;
        case 4:
            typeStr = "varchar";
            break;
        case 5:
            typeStr = "double";
            break;
        }
        ui->tableWidget->setItem(row,1,new QTableWidgetItem(typeStr));
        ui->tableWidget->item(row,1)->setFlags(Qt::ItemIsEnabled);
        //主键
        if(field.getIsPK()){
            QTableWidgetItem* qi = new QTableWidgetItem();
            qi->setIcon(QIcon(CHECK_PNG));
            ui->tableWidget->setItem(row,2,qi);
        }else{
            ui->tableWidget->setItem(row,2,new QTableWidgetItem(""));
        }
        ui->tableWidget->item(row,2)->setFlags(Qt::ItemIsEnabled);
        //允许为空
        if(!field.getIsNotNUll()){
            QTableWidgetItem* qi = new QTableWidgetItem();
            qi->setIcon(QIcon(CHECK_PNG));
            ui->tableWidget->setItem(row,3,qi);
        }else{
            ui->tableWidget->setItem(row,3,new QTableWidgetItem(""));
        }
        ui->tableWidget->item(row,3)->setFlags(Qt::ItemIsEnabled);
        //唯一
        if(field.getIsUnique()){
            QTableWidgetItem* qi = new QTableWidgetItem();
            qi->setIcon(QIcon(CHECK_PNG));
            ui->tableWidget->setItem(row,4,qi);
        }else{
            ui->tableWidget->setItem(row,4,new QTableWidgetItem(""));
        }
        ui->tableWidget->item(row,4)->setFlags(Qt::ItemIsEnabled);
        //如果是修改需要给值赋值
        if(recordEntity.getId()!=-1){
            QString value = recordEntity.getValue(field.getName());
            ui->tableWidget->setItem(row,5,new QTableWidgetItem(value));
        }else{
            //设置一个空字符串防止取值的时候空指针
            ui->tableWidget->setItem(row,5,new QTableWidgetItem(""));
        }
    }

}
