#include "tbcreatedialog.h"
#include "ui_tbcreatedialog.h"


TBCreateDialog::TBCreateDialog(QWidget *parent,QString databaseName,QString tableName) :
    QDialog(parent),
    ui(new Ui::TBCreateDialog)
{
    ui->setupUi(this);
    this->databaseName = databaseName;
    this->tableName = tableName;
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑
    //让tableWidget内容中的每个元素居中
    this->ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);//设置整行选中
    //信号与槽连接
    connect(this->ui->saveButton, SIGNAL(clicked(bool)), this, SLOT(save()));
    connect(this->ui->addButton, SIGNAL(clicked(bool)), this, SLOT(addField()));
    connect(ui->tableWidget,SIGNAL(cellClicked(int,int)),this,SLOT(setFieldEdit(int,int)));
    connect(ui->deleteButton,SIGNAL(clicked(bool)),this,SLOT(deleteField()));
    connect(ui->updateButton,SIGNAL(clicked(bool)),this,SLOT(modifyField()));
    init();
}

TBCreateDialog::~TBCreateDialog()
{
    delete ui;
}

vector<FieldEntity> TBCreateDialog::getFields() const
{
    return fields;
}

void TBCreateDialog::setFields(const vector<FieldEntity> &value)
{
    fields = value;
}

vector<FieldEntity> TBCreateDialog::getViewfields() const
{
    return viewfields;
}

void TBCreateDialog::setViewfields(const vector<FieldEntity> &value)
{
    viewfields = value;
}

vector<FieldEntity> TBCreateDialog::getCopyViewfields() const
{
    return copyViewfields;
}

void TBCreateDialog::setCopyViewfields(const vector<FieldEntity> &value)
{
    copyViewfields = value;
}

QString TBCreateDialog::getTableName() const
{
    return tableName;
}

void TBCreateDialog::setTableName(const QString &value)
{
    tableName = value;
}

void TBCreateDialog::setFieldEdit(int row, int col){
    // 获取所点击的单元格的字段的单元格
    QTableWidgetItem* item = ui->tableWidget->item(row ,0);
    this->ui->fieldNameEdit->setText(item->text());
}

void TBCreateDialog::init(){
    //判断是新建表还是修改表
    if(tableName.length()!=0){
        ui->tableNameEdit->setText(tableName);
        ui->tableNameEdit->setReadOnly(true);
        //不用查出现有数据库，只能使用本来的数据库
        this->ui->comboBox->addItem(this->databaseName);
        //进行一些按钮的隐藏
        ui->saveButton->hide();

    }else{
        //进行一些按钮的隐藏
        ui->fieldNameEdit->hide();
        ui->deleteButton->hide();
        ui->updateButton->hide();
        //首先查出现有数据库
        vector<DBEntity> dBList = DBService().getDBList();
        QString name = "";
        for(int i=0;i<dBList.size();i++){
            name = dBList.at(i).getName();
            this->ui->comboBox->addItem(name);
            if(name.compare(databaseName)==0){
                this->ui->comboBox->setCurrentIndex(i);
            }
        }
    }
    showStruct();
}
void TBCreateDialog::showStruct(){
    //判断一下有没有表名
    if(tableName.length()!=0){
        //查出表结构并
        vector<FieldEntity> fieldList = FieldService().getFieldList(databaseName,tableName);
        setViewfields(fieldList);
        //展示当前表的结构
        if(viewfields.size()!=0){
           for(int i=0;i<viewfields.size();i++){
               addItemToWidget(&(viewfields.at(i)));
           }
        }
    }

}

int TBCreateDialog::modifyField(){
    QString fieldName = this->ui->fieldNameEdit->text();
    //首先要根据字段名获取到字段实体
    FieldEntity fieldEntity;
    for(int i=0;i<viewfields.size();i++){
        if(viewfields.at(i).getName().compare(fieldName)==0){
            fieldEntity = viewfields.at(i);
            break;
        }
    }
    //然后将这个字段实体传给修改窗口
    FieldDialog* fieldDialog = new FieldDialog(databaseName,tableName,fieldName,&fieldEntity);
    connect(fieldDialog,SIGNAL(sendField(FieldEntity*)),this,SLOT(flushMofifyField(FieldEntity*)));
    fieldDialog->show();
}

void TBCreateDialog::flushMofifyField(FieldEntity *field){
    //进行重新xuan渲染
    ui->tableWidget->setRowCount(0);
    showStruct();
}

int TBCreateDialog::deleteField(){
    QString fieldName = this->ui->fieldNameEdit->text();
    if(fieldName.length()!=0){
        int result = FieldService().deleteField(ui->comboBox->currentText(),this->tableName,fieldName);
        if(result==1){
            //删除成功
            //重新进行显示
            //进行重新渲染
            ui->tableWidget->setRowCount(0);
            showStruct();
        }else{
            QMessageBox::warning(this,"错误","不存在该字段");
            return result;
        }
    }
    return 0;
}

int TBCreateDialog::save(){
    QString tableName = this->ui->tableNameEdit->text();
    QString databaseName = this->ui->comboBox->currentText();

    if(tableName.length()==0){
        QMessageBox::warning(nullptr,"警告","请输入表名称！");
        return -1;
    }

    //新建或修改表
    if(this->tableName.length()==0){//注意是this->tableName
        //说明是新建表
        TableService* tableService = new TableService();
        tableService->setDatabaseName(databaseName);
        int result = tableService->CreateTable(tableName,copyViewfields);
        free(tableService);


        //根据返回值，弹出不同提示
        if(result==0||result==1){
            emit sendData(databaseName,tableName);//发送信号，提示主窗口进行刷新
            this->close();
        }
        switch (result) {
        case INTEGER_TOO_LONG:
            QMessageBox::warning(nullptr,"警告","表名过长");
            break;
        case INTEGER_CONTAIN_SPACE:
            QMessageBox::warning(nullptr,"警告","表名包含空格");
            break;
        case TABLE_EXISTS:
            QMessageBox::warning(nullptr,"警告","该表已存在");
            break;
        }
        return result;
    }

}

void TBCreateDialog::addItemToWidget(FieldEntity* field){
    //显示新增的记录
    int count = this->ui->tableWidget->rowCount();
    ui->tableWidget->setRowCount(count+1);
    int i = 0;
    this->ui->tableWidget->setItem(count,i,new QTableWidgetItem(field->getName()));
    i++;
    int type = field->getType();
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
    this->ui->tableWidget->setItem(count,i,new QTableWidgetItem(typeStr));
    i++;
    this->ui->tableWidget->setItem(count,i,new QTableWidgetItem(QString::number(field->getLength())));
    i++;
    if(field->getMin()<=-100000000.0){
        this->ui->tableWidget->setItem(count,i,new QTableWidgetItem(""));
        i++;
        this->ui->tableWidget->setItem(count,i,new QTableWidgetItem(""));
        i++;
    }else{
        this->ui->tableWidget->setItem(count,i,new QTableWidgetItem(QString::number(field->getMin(),'f',3)));
        i++;
        this->ui->tableWidget->setItem(count,i,new QTableWidgetItem(QString::number(field->getMax(),'f',3)));
        i++;
    }

    this->ui->tableWidget->setItem(count,i,new QTableWidgetItem(field->getDefaultValue()));
    i++;
    if(field->getIsPK()){
        QTableWidgetItem* qi = new QTableWidgetItem();
        qi->setIcon(QIcon(CHECK_PNG));
        this->ui->tableWidget->setItem(count,i,qi);
        i++;
    }else{
        i++;
    }
    if(!field->getIsNotNUll()){
        QTableWidgetItem* qi = new QTableWidgetItem();
        qi->setIcon(QIcon(CHECK_PNG));
        this->ui->tableWidget->setItem(count,i,qi);
        i++;
    }else{
        i++;
    }
    if(field->getIsUnique()){
        QTableWidgetItem* qi = new QTableWidgetItem();
        qi->setIcon(QIcon(CHECK_PNG));
        this->ui->tableWidget->setItem(count,i,qi);
        i++;
    }else{
        i++;
    }
    this->ui->tableWidget->setItem(count,i,new QTableWidgetItem(field->getComment()));
}

void TBCreateDialog::flushAddField(FieldEntity* field){
    //把新增记录加入字段列表
    this->copyViewfields.push_back(*field);
    //再显示到视图上
    addItemToWidget(field);
}

void TBCreateDialog::addField(){
    //判断是修改还是新表
    if(tableName.length()!=0){
        FieldDialog* fieldDialog = new FieldDialog(databaseName,tableName);
        //绑定一个重新渲染的信号
        connect(fieldDialog,SIGNAL(sendField(FieldEntity*)),this,SLOT(flushMofifyField(FieldEntity*)));
        fieldDialog->show();
    }else{
        FieldDialog* fieldDialog = new FieldDialog();
        //信号与槽连接
        connect(fieldDialog, SIGNAL(sendField(FieldEntity*)), this, SLOT(flushAddField(FieldEntity*)));
        fieldDialog->show();
    }

}
