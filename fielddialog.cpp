#include "fielddialog.h"
#include "ui_fielddialog.h"
#include "global.h"
#include "recordservice.h"

FieldDialog::FieldDialog(QString dbName,QString tableName,QString oldFieldName,FieldEntity* fieldEntity,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FieldDialog)
{
    ui->setupUi(this);
    this->fieldEntity = fieldEntity;
    this->dbName = dbName;
    this->tableName = tableName;
    this->oldFieldName = oldFieldName;
    init();
    connect(this->ui->savaButton, SIGNAL(clicked(bool)), this, SLOT(save()));
    connect(this->ui->cancelButton, SIGNAL(clicked(bool)), this, SLOT(cancel()));
    //检测combox的改变
    connect(this->ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(reSetBasic()));
    //检测选择框
    connect(ui->PKcheckBox,SIGNAL(stateChanged(int)),this,SLOT(checkPK(int)));
    connect(ui->nullcheckBox,SIGNAL(stateChanged(int)),this,SLOT(checkNull(int)));
    connect(ui->suoyinBox,SIGNAL(stateChanged(int)),this,SLOT(checkSuoYin(int)));
}

FieldDialog::~FieldDialog()
{
    delete ui;
}

void FieldDialog::save(){
    //判断是新增还是修改
    if(this->fieldEntity==NULL&&tableName.length()==0){
        //检查字段名
        QString fieldName = this->ui->nameEdit->text();
        if(FileUtil().checkFieldName(fieldName)!=YES){
            //进行相应消息提示
            QMessageBox::warning(this,"错误","字段名不合法");
            return;
        }
        FieldEntity* field = new FieldEntity();
        field->setName(fieldName);
        //获取字段类型
        int fieldType = this->ui->comboBox->currentIndex()+1;
        if(dealFieldType(fieldType)){
            //设置type
            field->setType(fieldType);
            //获取用户其他选项
            //长度
            if(fieldType == 4){
                field->setLength(this->ui->lengthEdit->text().toInt());
            }else{
                field->setLength(0);
            }
            //默认值
            QString defaulValue = this->ui->defaultEdit->text();
            if(defaulValue.length()!=0){
                field->setDefaultValue(defaulValue);
            }
            //注释
            QString explain = this->ui->explainEdit->text();
            if(explain.length()!=0){
                field->setComment(explain);
            }
            //最大最小值
            QString min = this->ui->minEdit->text();
            if(min.length()!=0){
                field->setMin(min.toDouble());
                field->setMax(this->ui->maxEdit->text().toDouble());
            }else{
                field->setMin(-100000000.0);
                field->setMax(100000000.0);
            }
            //是否为主键
            if(this->ui->PKcheckBox->isChecked()){
                field->setIsPK(1);
            }
            //是否允许空值
            if(this->ui->nullcheckBox->isChecked()){
                field->setIsNotNUll(0);
            }
            //是否唯一
            if(this->ui->uniquecheckBox->isChecked()){
                field->setIsUnique(1);
            }
            //是否创建索引
            if(this->ui->suoyinBox->isChecked()){
                field->setIsSuoYin(1);
            }
            emit sendField(field);
            this->close();
        }
    }else{
        //修改
        //检查字段名
        QString fieldName = this->ui->nameEdit->text();
        if(FileUtil().checkFieldName(fieldName)!=YES){
            //进行相应消息提示
            QMessageBox::warning(this,"错误","字段名不合法");
            return;
        }
        FieldEntity* field = new FieldEntity();
        field->setName(fieldName);
        //获取字段类型
        int fieldType = this->ui->comboBox->currentIndex()+1;
        if(dealFieldType(fieldType)){
            //设置type
            field->setType(fieldType);
            //获取用户其他选项
            //长度
            if(fieldType == 4){
                field->setLength(this->ui->lengthEdit->text().toInt());
            }else{
                field->setLength(0);
            }
            //默认值
            QString defaulValue = this->ui->defaultEdit->text();
            if(defaulValue.length()!=0){
                field->setDefaultValue(defaulValue);
            }else{
                 field->setDefaultValue("null");
            }
            //注释
            QString explain = this->ui->explainEdit->text();
            if(explain.length()!=0){
                field->setComment(explain);
            }else{
                field->setComment("");
            }
            //最大最小值
            QString min = this->ui->minEdit->text();
            if(min.length()!=0){
                field->setMin(min.toDouble());
                field->setMax(this->ui->maxEdit->text().toDouble());
            }else{
                field->setMin(-100000000.0);
                field->setMax(100000000.0);
            }
            //是否为主键
            if(this->ui->PKcheckBox->isChecked()){
                field->setIsPK(1);
            }
            //是否允许空值
            if(this->ui->nullcheckBox->isChecked()){
                field->setIsNotNUll(0);
            }
            //是否唯一
            if(this->ui->uniquecheckBox->isChecked()){
                field->setIsUnique(1);
            }
            //是否创建索引
            if(this->ui->suoyinBox->isChecked()){
                field->setIsSuoYin(1);
                QString path = DB_CREATE_PATH;
                path.append(dbName + "//" + tableName + "//");
                solveSuoYin(path,field);
            }else{
                field->setIsSuoYin(0);
                QString path = DB_CREATE_PATH;
                path.append(dbName + "//" + tableName + "//");
                solveSuoYin(path,field);
            }
            //判断一下是给表加字段还是改字段
            
            if(oldFieldName.length()==0){
                int result = FieldService().insertField(dbName,tableName,*field);
                if(result !=1){
                    QMessageBox::warning(nullptr,"错误","添加新字段失败");
                }
                
            }else{
                int result = FieldService().modifyField(dbName,tableName,oldFieldName,*field);
                if(result !=1){
                    QMessageBox::warning(nullptr,"错误","修改字段失败");
                }
            }
            emit sendField(field);
            this->close();

     }

    }
}

void FieldDialog::cancel(){
    this->close();
}
bool FieldDialog::dealFieldType(int type){
    //如果是varchar,则必须指定长度
    if(type==VARCHAR){
        if(this->ui->lengthEdit->text().length()==0){
            QMessageBox::warning(this,"错误","varchar类型必须指定长度");
            return false;
        }
    }
    //如果是int和type
    if(type == INT || type==DOUBLE){
        //检查最大最小值是不是只填了一个
        QString min = this->ui->minEdit->text();
        QString max = this->ui->maxEdit->text();
        if(min.length()==0&&max.length()==0){
            return true;
        }
        if(min.length()!=0&&max.length()!=0){
            return true;
        }
        QMessageBox::warning(this,"错误","最大最小值必须同时填写");
        return false;
    }
    return true;
}

void FieldDialog::init(){

    //给combobox设置值
    this->ui->comboBox->addItem("int");
    this->ui->comboBox->addItem("bool");
    this->ui->comboBox->addItem("time");
    this->ui->comboBox->addItem("varchar");
    this->ui->comboBox->addItem("double");

    //如果是修改设置默认值
    if(fieldEntity!=NULL){     
        ui->comboBox->setCurrentIndex(fieldEntity->getType()-1);

        if(fieldEntity->getDefaultValue().compare("null")!=0){
           ui->defaultEdit->setText(fieldEntity->getDefaultValue());
        }
        ui->explainEdit->setText(fieldEntity->getComment());

        if(ui->comboBox->currentIndex()!=3){
            ui->lengthEdit->setReadOnly(true);
        }else{
            ui->lengthEdit->setText(QString::number(fieldEntity->getLength()));

        }
        if(fieldEntity->getType()!=1&&fieldEntity->getType()!=5){
            ui->maxEdit->setReadOnly(true);
            ui->minEdit->setReadOnly(true);
        }else{
            if(fieldEntity->getMin()>-100000000.0){
                ui->maxEdit->setText(QString::number(fieldEntity->getMin(),'f',3));
                ui->minEdit->setText(QString::number(fieldEntity->getMax(),'f',3));
            }
        }
        ui->nameEdit->setText(fieldEntity->getName());
        ui->nullcheckBox->setChecked(!fieldEntity->getIsNotNUll());
        ui->PKcheckBox->setChecked(fieldEntity->getIsPK());
        ui->uniquecheckBox->setChecked(fieldEntity->getIsUnique());
        //是修改的话要限制不可以更改类型
        ui->comboBox->setEnabled(false);
    }else{
        this->ui->lengthEdit->setEnabled(false);
    }
}

void FieldDialog::reSetBasic(){
    //获取到当前字符串
    QString text = ui->comboBox->currentText();

    //对长度输入框的设置
    if(text.compare("varchar")==0){
        ui->lengthEdit->clear();
        ui->lengthEdit->setEnabled(true);
    }else{
        ui->lengthEdit->clear();
        ui->lengthEdit->setEnabled(false);
    }

    //对最大最小值输入框的设置
    if(text.compare("int")==0 || text.compare("double")==0){
        ui->minEdit->clear();
        ui->maxEdit->clear();
        ui->minEdit->setEnabled(true);
        ui->maxEdit->setEnabled(true);
    }else{
        ui->minEdit->clear();
        ui->maxEdit->clear();
        ui->minEdit->setEnabled(false);
        ui->maxEdit->setEnabled(false);
    }
    //对主键，唯一的设置
    if(text.compare("bool")==0){
        ui->PKcheckBox->setChecked(false);
        ui->PKcheckBox->setEnabled(false);
        ui->uniquecheckBox->setChecked(false);
        ui->uniquecheckBox->setEnabled(false);
        ui->suoyinBox->setChecked(false);
         ui->suoyinBox->setEnabled(false);
    }else{
        ui->PKcheckBox->setChecked(false);
        ui->PKcheckBox->setEnabled(true);
        ui->uniquecheckBox->setChecked(false);
        ui->uniquecheckBox->setEnabled(true);
        ui->suoyinBox->setChecked(false);
         ui->suoyinBox->setEnabled(true);
    }
}

void FieldDialog::checkPK(int state){
    if(state == 2){
        ui->nullcheckBox->setChecked(false);
        ui->nullcheckBox->setCheckable(false);
        ui->uniquecheckBox->setChecked(true);
    }else{
        ui->nullcheckBox->setChecked(false);
        ui->nullcheckBox->setCheckable(true);
    }
}

void FieldDialog::checkNull(int state){
    if(state == 2){
        ui->PKcheckBox->setChecked(false);
        ui->PKcheckBox->setCheckable(false);
    }else{
        ui->PKcheckBox->setChecked(false);
        ui->PKcheckBox->setCheckable(true);
    }
}

void FieldDialog::checkSuoYin(int state){
    if(state == 2){
        ui->uniquecheckBox->setChecked(true);
    }
}

void  FieldDialog::solveSuoYin(QString path,FieldEntity *field){
    QString path_record = path;
    path.append(field->getName() + ".findex");
    if(field->getIsSuoYin()){
        if(!FileUtil().ifExistFile(path)){
            //先查出所有记录
            vector<RecordEntity> recordList = RecordService().getRecordList(dbName,tableName);
            //判断一下字段类型
            if(field->getType()==VARCHAR){

            }else{

            }
        }
    }else{
        if(FileUtil().ifExistFile(path)){
            FileUtil().deleteFile(path);
        }
    }
}
