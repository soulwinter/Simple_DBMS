#include "dialog.h"
#include "ui_dialog.h"
#include "fileutil.h"
#include <QMenu>
#include <QFileDialog>

Dialog::Dialog(QString username,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    this->username = username;
    ui->setupUi(this);
    this->ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);//不可编辑
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //信号与槽连接
    connect(this->ui->pushButton, SIGNAL(clicked(bool)), this, SLOT(addItem()));
    connect(this->ui->addTableButton, SIGNAL(clicked(bool)), this, SLOT(addTable()));
    connect(this->ui->deleteRecordButton,SIGNAL(clicked(bool)),this,SLOT(deleteRecord()));
    connect(this->ui->updateRecordButton,SIGNAL(clicked(bool)),this,SLOT(updateRecord()));
    connect(this->ui->testButton, SIGNAL(clicked(bool)), this, SLOT(inputDB()));
    connect(this->ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*,int)), this, SLOT(changeDatabaseEdit(QTreeWidgetItem*,int)));
    connect(this->ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(rightClickMenu(QTreeWidgetItem*)));
    connect(this->ui->tableWidget,SIGNAL(itemClicked(QTableWidgetItem*)),this,SLOT(doubleClickRecord(QTableWidgetItem*)));
    connect(this->ui->dealButton,SIGNAL(clicked(bool)),this,SLOT(doSql()));
//    connect(dBcreateDialog, SIGNAL(sendData(QString)), this, SLOT(showDatabase(QString))); 不能放在这绑定，还没有初始化
    init();

}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::doSql(){
    QString input = this->ui->textEdit->toPlainText();
    sqlAnalyze* sql = new sqlAnalyze(this->ui->DBlineEdit->text());
    if(this->ui->DBlineEdit->text().length()==0){
        QMessageBox::about(this,"警告","未选择数据库!");
        return;
    }
    if(input.contains("select")){
        //这里分三种，一种是普通查询，一种是仅包含聚组函数，一种是多表联查。
        //1.普通查询情况
        vector<RecordEntity> result;
        if(input.contains("join")){
            result = sql->selectSqlWithJoin(this->ui->DBlineEdit->text(),input);
        }else if(input.contains("sum") || input.contains("avg") || input.contains("max") || input.contains("min") || input.contains("count")){
            result = sql->selectSqlsum(this->ui->DBlineEdit->text(),input);
        }else{
            result = sql->selectSql(this->ui->DBlineEdit->text(),input);
        }
        if(result.size()==0){
            ui->label_3->setText("语法错误");
            return;
        }
        RecordEntity statusRecord = result.at(result.size()-1);
        QString status = statusRecord.getValue("status");
        if(status.compare("200")==0){
            if(input.contains("join")){
                RecordEntity viewRecord = result.at(result.size()-2);
                //遍历取得需要展示的字段
                vector<QString> needView;
                map<QString, QString> valueMap = viewRecord.getValueMap();
                map<QString, QString>::iterator iter;
                iter = valueMap.begin();
                while(iter != valueMap.end()) {
                    needView.push_back(iter->first);
                    iter++;
                }
                //进行展示
                flushSelectViewSelective(result,needView);
            }else if(input.contains("sum") || input.contains("avg") || input.contains("max") || input.contains("min")){
                RecordEntity viewRecord = result.at(result.size()-2);
                //遍历取得需要展示的字段
                vector<QString> needView;
                map<QString, QString> valueMap = viewRecord.getValueMap();
                map<QString, QString>::iterator iter;
                iter = valueMap.begin();
                while(iter != valueMap.end()) {
                    needView.push_back(iter->first);
                    iter++;
                }
                //进行展示
                flushSelectViewSelective(result,needView);
            }else{
                RecordEntity viewRecord = result.at(result.size()-2);
                QString tableName = statusRecord.getValue("tableName");
                vector<FieldEntity> fieldList = FieldService().getFieldList(this->ui->DBlineEdit->text(),tableName);
                //遍历取得需要展示的字段
                vector<QString> needView;
                map<QString, QString> valueMap = viewRecord.getValueMap();
                map<QString, QString>::iterator iter;
                iter = valueMap.begin();
                while(iter != valueMap.end()) {
                    if(iter->first.compare("*")==0){
                        for(int m=0;m<fieldList.size();m++){
                            needView.push_back(fieldList.at(m).getName());
                        }
                        break;
                    }else{
                        needView.push_back(iter->first);
                    }
                    iter++;
                }
                //进行展示
                flushSelectViewSelective(result,needView);
            }

        }else{
            if(status.compare("400")==0){
                ui->label_3->setText("语法错误");
            }
            if(status.compare("401")==0){
                ui->label_3->setText("不存在字段");
            }
            if(status.compare("402")==0){
                ui->label_3->setText("不存在表");
            }

        }

    }else{
        int result = sql->checkInput(input);
        if(result){
            if(result == 1){
                ui->label_3->clear();
                flush();
            }else{
                if(result == 8009){
                    ui->label_3->setText("检查一下，某个字段不允许为空");
                }
                if(result == 8008){
                    ui->label_3->setText("检查一下，某个字段长度溢出");
                }
                if(result == 8007){
                    ui->label_3->setText("检查一下，某个字段超出范围");
                }
                if(result == 8005){
                    ui->label_3->setText("检查一下，主键或唯一重复");
                }
                if(result == 8010){
                    ui->label_3->setText("检查一下，字段不存在");
                }
                if(result == 9001 || result == 313 || result == 103){
                    ui->label_3->setText("不存在该表");
                }
                if(result == 9002){
                    ui->label_3->setText("只要int和double类型允许+-*/");
                }
                if(result == -1){
                    ui->label_3->setText("不能除以0");
                }
                if(result == 317 || result == 102){
                    ui->label_3->setText("同学，请检查你写的sql语句哟");
                }
                if(result == 500){
                    ui->label_3->setText("数据库已存在或名称不合法");
                }
                if(result == 999){
                    ui->label_3->setText("不存在字段类型");
                }
                if(result == 301){
                    ui->label_3->setText("表已存在");
                }
                if(result == 10086){
                    ui->label_3->setText("字段类型不匹配");
                }
                if(result == 10087){
                    ui->label_3->setText("修改字段类型错误：null");
                }
                if(result == 10088){
                    ui->label_3->setText("修改字段类型错误：length");
                }
                if(result == 10089){
                    ui->label_3->setText("修改字段类型错误：max or min");
                }
                if(result == 10090){
                    ui->label_3->setText("修改字段类型错误: PK or unique");
                }

            }
        }else{
            if(input.contains("rename") && input.contains("to")){
                ui->label_3->clear();
                flush();
                return;
            }
            ui->label_3->setText("同学，请检查你写的sql语句哟");
        }
    }

}

void Dialog::addItem(){
    dBcreateDialog = new DBCreateDiqlog();
    connect(dBcreateDialog, SIGNAL(sendData(QString)), this, SLOT(showDatabase(QString)));
    dBcreateDialog->show();
}

//新建表
void Dialog::addTable(){
    tbcreateDialog = new TBCreateDialog(this,this->ui->DBlineEdit->text());
    connect(tbcreateDialog, SIGNAL(sendData(QString,QString)), this, SLOT(flushTable(QString,QString)));
    tbcreateDialog->show();

}

//修改表结构
void Dialog::modifyTable(){
    TBCreateDialog* tbmodifyDialog = new TBCreateDialog(this,curItem->parent()->text(0),curItem->text(0));
    tbmodifyDialog->show();
}

void Dialog::flushTable(QString dbName,QString tableName){
    QTreeWidgetItemIterator it(ui->treeWidget);
    while(*it){
        QString databaseName = (*it)->text(0);
        QString type = (*it)->text(1);
        if(type.compare("D")==0&&databaseName.compare(dbName)==0){
            //找到对应的数据库给他加一个表
            QTreeWidgetItem* child = new QTreeWidgetItem();
            child->setText(0,tableName);
            child->setText(1,"T");//标识位
            child->setIcon(0,QIcon("/Users/soulwinter/Downloads/table.png"));
            (*it)->addChild(child);
            break;
        }
        ++it;
    }
}

void Dialog::flush(){
    this->ui->treeWidget->clear();
    init();
}

void Dialog::showDatabase(QString databaseName){
    //将用户新建的数据库存入用户信息
    QString oldContent;
    QString newContent;
    if(username.compare("root")!=0){
        QString path = DB_CREATE_PATH;
        path.append("user.info");
        if(FileUtil().ifExistFile(path)){
            int num = FileUtil().getCounter(path);
            for(int i = 1;i <= num+1 ; i++){
                QString content = FileUtil().readLine(path,i);
                QStringList localSplit = content.split(" ");
                if(localSplit[0].compare(username) == 0){
                    oldContent = content;
                    newContent = content;
                    newContent.append(" ").append(databaseName);
                    FileUtil().replaceContent(path,oldContent,newContent);
                }
            }
        }
    }

    QTreeWidgetItem* item = new QTreeWidgetItem();
    item->setText(0,databaseName);
    item->setText(1,"D");//标识位，不会显示，用于点击时区分是数据库还是表
    item->setIcon(0,QIcon("/Users/soulwinter/Downloads/database.PNG"));
    this->ui->treeWidget->addTopLevelItem(item);
}


void Dialog::test(){
      QString x = FileUtil().readLine("/Users/soulwinter/Desktop/DBMSData/system.db",2);
      int i = 1;
}

void Dialog::changeDatabaseEdit(QTreeWidgetItem* item,int column){
    if(item->text(1).compare("D")==0){
        this->ui->DBlineEdit->setText(item->text(0));
        this->ui->TBlineEdit->setText("");
    }else{
        this->ui->TBlineEdit->setText(item->text(0));
        //同时将数据库输入框设置为表所在的数据库
        this->ui->DBlineEdit->setText(item->parent()->text(0));
    }
}

QStringList Dialog::getUserInfoByUsername(QString username){
    QString path = DB_CREATE_PATH;
    path.append("user.info");
    if(FileUtil().ifExistFile(path)){
        int counter = FileUtil().getCounter(path);
        for(int i = 2;i <=counter+1;i++){
            QString content = FileUtil().readLine(path,i);
            QStringList localSplit = content.split(' ');
            if(localSplit[0].compare(username)==0){
                return localSplit;
            }
        }
    }
    QStringList ss;
    return ss;
}

void Dialog::init(){
    //读取现存数据库
    DBService* dbService = new DBService();
    vector<DBEntity> dBListInit = dbService->getDBList();
    vector<DBEntity> dBList;

    //只显示该用户的数据库
    if(username.compare("root")!=0){
        QStringList userInfoByUsername = getUserInfoByUsername(username);
        for(int i = 0;i<dBListInit.size();i++){
            DBEntity db = dBListInit.at(i);
            for(int j=2;j<userInfoByUsername.size();j++){
                if(db.getName().compare(userInfoByUsername[j])==0){
                    dBList.push_back(db);
                    break;
                }
            }
        }
    }else{
        dBList = dBListInit;
    }

    for(int i = 0;i<dBList.size();i++){
        DBEntity dBEntity = dBList.at(i);
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0,dBEntity.getName());
        item->setText(1,"D");//标识位，不会显示，用于点击时区分是数据库还是表
        item->setIcon(0,QIcon("/Users/soulwinter/Downloads/database.PNG"));
        addTableChildByDB(dBEntity,item);
        this->ui->treeWidget->addTopLevelItem(item);
    }
    ui->treeWidget->setIconSize(QSize(15,15));
}

void Dialog::addTableChildByDB(DBEntity dbEntity,QTreeWidgetItem* item){ //根据数据库展示表
    vector<TableEntity> tBList = TableService().getTBListByDBName(dbEntity.getName());
    for(int i=0;i<tBList.size();i++){
        QTreeWidgetItem* child = new QTreeWidgetItem();
        child->setText(0,tBList.at(i).getTableName());
        child->setText(1,"T");//标识位
        child->setIcon(0,QIcon("/Users/soulwinter/Downloads/table.png"));
        item->addChild(child);
    }
}

void Dialog::deleteItem(){
    if(curItem->text(1).compare("D")==0){
        if(QMessageBox::Yes == QMessageBox::question(nullptr, "请确认", "确认删除数据库吗?")){
            DBService().deleteDatabase(curItem->text(0));
            QString toDeleteDB = curItem->text(0);
            flush();
            //从user.info里删除数据库信息
            QStringList userInfoByUsername = getUserInfoByUsername(username);
            QString oldContent;
            oldContent.append(userInfoByUsername.at(0));
            for(int i=1;i < userInfoByUsername.size();i++){
                oldContent.append(" ").append(userInfoByUsername.at(i));
            }
            QString newContent;
            newContent.append(username).append(" ").append(userInfoByUsername[1]);
            for(int i = 2;i < userInfoByUsername.size();i++){
                if(userInfoByUsername.at(i).compare(toDeleteDB)!=0){
                    newContent.append(" ").append(userInfoByUsername.at(i));
                }
            }
            QString path = DB_CREATE_PATH;
            path.append("user.info");
            FileUtil().replaceContent(path,oldContent,newContent);
        }
    }else{
        if(QMessageBox::Yes == QMessageBox::question(nullptr, "请确认", "确认删除表吗?")){
            TableService* tableService = new TableService();
            tableService->setDatabaseName(curItem->parent()->text(0));
            tableService->DeleteTable(curItem->text(0));
            flush();
        }
    }

}

void Dialog::modifyItemName(){//修改数据库或者表的名字
    if(curItem->text(1).compare("D")==0){
        TBModifyNameDialog* modifyDialog = new TBModifyNameDialog(curItem->text(0),curItem->text(0),1);
        connect(modifyDialog,SIGNAL(sendFlag()),this,SLOT(flush()));
        modifyDialog->show();
    }else{
        TBModifyNameDialog* modifyDialog = new TBModifyNameDialog(curItem->parent()->text(0),curItem->text(0),0);
        connect(modifyDialog,SIGNAL(sendFlag()),this,SLOT(flush()));
        modifyDialog->show();
    }
}

//新增记录
void Dialog::addRecord(){
    RecordEntity record;
    record.setId(-1);
    RecordCreateDialog* recordDialog = new RecordCreateDialog(record,curItem->parent()->text(0),curItem->text(0));
    recordDialog->show();
}

//删除记录
void Dialog::deleteRecord(){
    if(!curTableItem==NULL){
        if(deleteDbName.length()==0 || deleteTableName.length()==0){
            QMessageBox::about(this,"警告","当前删除操作违法！");
            return;
        }
        if(QMessageBox::Yes == QMessageBox::question(nullptr, "请确认", "确认删除该条记录吗?")){
            //获取到当前表有几个字段
            int col = ui->tableWidget->columnCount();
            //获取到点击的row
            int row = curTableItem->row();
            //声明一个记录实体
            RecordEntity record;
            record.setId(row+1);
            for(int i = 0;i<col;i++){
                QString fieldName = ui->tableWidget->horizontalHeaderItem(i)->text();
                QString value = ui->tableWidget->item(row,i)->text();
                record.SetValue(fieldName,value);
            }
            int result = RecordService().deletrecord(record,deleteDbName,deleteTableName);
            if(result == 1){
                QMessageBox::about(this,"成功","删除成功");
            }else{
                QMessageBox::about(this,"失败","删除失败");
            }
        }
        openTable();
    }else{
        QMessageBox::warning(nullptr, "警告", "未选择记录!");
    }

}

//修改记录
void Dialog::updateRecord(){
    if(curTableItem==NULL){
        QMessageBox::warning(nullptr, "警告", "未选择记录!");
    }else{
        if(deleteDbName.length()==0 || deleteTableName.length()==0){
            QMessageBox::about(this,"警告","当前修改操作违法！");
            return;
        }
        //获取到当前表有几个字段
        int col = ui->tableWidget->columnCount();
        //获取到点击的row
        int row = curTableItem->row();
        //声明一个记录实体
        RecordEntity record;
        record.setId(row+1);
        for(int i = 0;i<col;i++){
            QString fieldName = ui->tableWidget->horizontalHeaderItem(i)->text();
            QString value = ui->tableWidget->item(row,i)->text();
            record.SetValue(fieldName,value);
        }
        RecordCreateDialog* recordDialog = new RecordCreateDialog(record,deleteDbName,deleteTableName);
        recordDialog->show();
    }
}

//查询窗口
void Dialog::selectRecord(){
    QString dbName = curItem->parent()->text(0);
    QString tableName = curItem->text(0);
    SelectDialog *localSelectDialog = new SelectDialog(dbName,tableName);
    //绑定信号，用于回显
    connect(localSelectDialog,SIGNAL(sendSelectInfo(vector<RecordEntity>)),this,SLOT(flushSelectView(vector<RecordEntity>)));
    localSelectDialog->show();
}

void Dialog::flushSelectViewSelective(vector<RecordEntity> result, vector<QString> needView){
    //首先清空
    ui->tableWidget->clear();
    //修改当前点击表项
    curTableItem = NULL;
    //进行显示
    ui->tableWidget->setColumnCount(needView.size());
    ui->tableWidget->setRowCount(result.size()-2);
    QStringList list;
    for(int i=0;i<needView.size();i++){
        list << needView.at(i);
    }
    ui->tableWidget->setHorizontalHeaderLabels(list);
    for(int i=0;i<result.size()-2;i++){
        RecordEntity record = result.at(i);
        for(int j = 0;j<needView.size();j++){
            ui->tableWidget->setItem(i,j,new QTableWidgetItem(record.getValue(needView.at(j))));
            ui->tableWidget->item(i,j)->setFlags(Qt::ItemIsEnabled);
        }
    }
}

void Dialog::flushSelectView(vector<RecordEntity> result){
    //首先清空
    ui->tableWidget->clear();

    //修改当前点击表项
    curTableItem = NULL;

    //获取数据
    vector<FieldEntity> fieldList = FieldService().getFieldList(curItem->parent()->text(0),curItem->text(0));

    //进行显示
    ui->tableWidget->setColumnCount(fieldList.size());
    ui->tableWidget->setRowCount(result.size());
    QStringList list;
    for(int i=0;i<fieldList.size();i++){
        list << fieldList.at(i).getName();
    }
    ui->tableWidget->setHorizontalHeaderLabels(list);
    for(int i=0;i<result.size();i++){
        RecordEntity record = result.at(i);
        for(int j = 0;j<fieldList.size();j++){
            FieldEntity field = fieldList.at(j);
            ui->tableWidget->setItem(i,j,new QTableWidgetItem(record.getValue(field.getName())));
            ui->tableWidget->item(i,j)->setFlags(Qt::ItemIsEnabled);
        }
    }
}

//打开表，即展示所有记录
void Dialog::openTable(){
    //首先清空
    ui->tableWidget->clear();

    //给删除用的数据库名和表名重新赋值
    deleteDbName = curItem->parent()->text(0);
    deleteTableName = curItem->text(0);

    //获取数据
    vector<RecordEntity> recordList = RecordService().getRecordList(curItem->parent()->text(0),curItem->text(0));
    vector<FieldEntity> fieldList = FieldService().getFieldList(curItem->parent()->text(0),curItem->text(0));

    //进行显示
    ui->tableWidget->setColumnCount(fieldList.size());
    ui->tableWidget->setRowCount(recordList.size());
    QStringList list;
    for(int i=0;i<fieldList.size();i++){
        list << fieldList.at(i).getName();
    }
    ui->tableWidget->setHorizontalHeaderLabels(list);
    for(int i=0;i<recordList.size();i++){
        RecordEntity record = recordList.at(i);
        for(int j = 0;j<fieldList.size();j++){
            FieldEntity field = fieldList.at(j);
            ui->tableWidget->setItem(i,j,new QTableWidgetItem(record.getValue(field.getName())));
        }
    }
}

void Dialog::doubleClickRecord(QTableWidgetItem *item){
    this->curTableItem = item;
}

void Dialog::rightClickMenu(QTreeWidgetItem* item){//弹出菜单
    this->curItem = item; //修改当前用户选中的item
    //根据点击的类型弹出菜单
    if(item->text(1).compare("D")==0){
        //点击数据库
        QMenu* q = new QMenu();
        QAction * action1 = new QAction(tr("删除数据库"),this);
        connect(action1,SIGNAL(triggered(bool)),this,SLOT(deleteItem()));
        q->addAction(action1);
        QAction * action2 = new QAction(tr("修改数据库名"),this);
        connect(action2,SIGNAL(triggered(bool)),this,SLOT(modifyItemName()));
        q->addAction(action2);
        QAction * action3 = new QAction(tr("导出数据库"),this);
        connect(action3,SIGNAL(triggered(bool)),this,SLOT(outputDB()));
        q->addAction(action3);
        
        
        q->exec(QCursor::pos());
        q->show();
        q->close();
    }else{
        //点击表
        QMenu* q = new QMenu();
        QAction * action1 = new QAction(tr("删除表"),this);
        connect(action1,SIGNAL(triggered(bool)),this,SLOT(deleteItem()));
        q->addAction(action1);
        QAction * action2 = new QAction(tr("修改表名"),this);
        connect(action2,SIGNAL(triggered(bool)),this,SLOT(modifyItemName()));
        q->addAction(action2);
        QAction * action3 = new QAction(tr("修改表结构"),this);
        connect(action3,SIGNAL(triggered(bool)),this,SLOT(modifyTable()));
        q->addAction(action3);
        QAction * action4 = new QAction(tr("添加记录"),this);
        connect(action4,SIGNAL(triggered(bool)),this,SLOT(addRecord()));
        q->addAction(action4);
        QAction * action5 = new QAction(tr("打开表"),this);
        connect(action5,SIGNAL(triggered(bool)),this,SLOT(openTable()));
        q->addAction(action5);
        QAction * action6 = new QAction(tr("条件查询"),this);
        connect(action6,SIGNAL(triggered(bool)),this,SLOT(selectRecord()));
        q->addAction(action6);


        q->exec(QCursor::pos());
        q->show();
        q->close();
    }
}


void Dialog::outputDB(){
    QString dbName = curItem->text(0);
    if(dbName.length()!=0){
        //导出路径
        QString path = DB_CREATE_PATH;
        Export* var1 = new Export(dbName,path);
        var1->createFile();
    }else{
        QMessageBox::warning(nullptr,"警告","请选择要导出的数据库");
    }
}

void Dialog::inputDB(){
    QString path = QFileDialog::getOpenFileName(this,tr(""),"C:\\",tr("")); //选择路径
    //判断文件后缀是否正确
    QString check = path.right(3);
    if(check.compare("sql")==0){
        if(Export().importFile(path)){
            //所有逻辑完成刷新
            flush();
        }else{
            QMessageBox::warning(nullptr,"警告","导入失败");
        }

    }else{
         QMessageBox::warning(nullptr,"警告","文件格式不正确");
    }
}
