#include "selectdialog.h"
#include "ui_selectdialog.h"
#include<vector>

SelectDialog::SelectDialog(QString dbName,QString tableName,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectDialog)
{
    this->dbName = dbName;
    this->tableName = tableName;
    ui->setupUi(this);
    init();
    connect(ui->cancelButton,SIGNAL(clicked(bool)),this,SLOT(cancel()));
    connect(ui->selectButton,SIGNAL(clicked(bool)),this,SLOT(select()));
}

SelectDialog::~SelectDialog()
{
    delete ui;
}

void SelectDialog::cancel(){
    this->close();
}

void SelectDialog::select(){
    map<QString,QString> valueMap; //用于保存用户输入
    //遍历用户输入
    int rowCount = ui->tableWidget->rowCount();
    for(int i = 0;i < rowCount;i++){
        QString localText = ui->tableWidget->item(i,1)->text();
        if(localText.length()!=0){ //有输入才填入
            QString fieldName = ui->tableWidget->item(i,0)->text();
            valueMap.insert(make_pair(fieldName,localText));
        }
    }
    //进行查询
    vector<RecordEntity> result;
    vector<RecordEntity> recordList = RecordService().getRecordList(dbName,tableName);
    for(int i = 0;i < recordList.size();i++){
        RecordEntity record = recordList.at(i);
        //遍历valueMap进行比较
        int flag = 0; //标识看有没有不相等的

        for(map<QString,QString>::iterator  iter=valueMap.begin();iter!=valueMap.end();iter++){
            QString key = iter->first;
            QString val = iter->second;
            if(record.getValue(key).compare(val)!=0){
                flag = 1;
                break;
            }
         }
        if(!flag){
            result.push_back(record);
        }
    }

    emit sendSelectInfo(result);
    this->close();
}

void SelectDialog::init(){
    //获取到各个字段
    vector<FieldEntity> fieldList = FieldService().getFieldList(dbName,tableName);
    if(fieldList.size()==0) return;

    ui->tableWidget->setRowCount(fieldList.size());
    //遍历fieldList显示字段名
    for(int i = 0;i < fieldList.size();i++){
        FieldEntity field = fieldList.at(i);
        this->ui->tableWidget->setItem(i,0,new QTableWidgetItem(field.getName()));
        ui->tableWidget->item(i,0)->setFlags(Qt::ItemIsEnabled);
        //给值设置空字符串，防止后面空指针
        ui->tableWidget->setItem(i,1,new QTableWidgetItem(""));
    }
}
