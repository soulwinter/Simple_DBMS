#include "tableservice.h"


TableService::TableService()
{

}

QString TableService::getDatabaseName() const
{
    return databaseName;
}

void TableService::setDatabaseName(const QString &value)
{
    databaseName = value;
}

int TableService::checkField(FieldEntity field){
    QString name = field.getName();
    //不为空
    if(name.length()==0){
        return STRING_NULL;
    }
    if(!FileUtil().ifContainsSpace(name)){
        return INTEGER_CONTAIN_SPACE;
    }
    return YES;
}

int TableService::CreateTable(QString tableName,vector<FieldEntity>fields){ //根据表名，和字段来创建表
    //对表名的合法性进行检测
    if(tableName.length()>128){
        return INTEGER_TOO_LONG;
    }
    if(tableName.length()==0){
        return STRING_NULL;
    }
    if(!FileUtil().ifContainsSpace(tableName)){
        return INTEGER_CONTAIN_SPACE;
    }
    //表检查通过，对字段名检查
    if(!fields.empty()){
        FieldEntity field;
        for(int i=0;i<fields.size();i++){
            field = fields.at(i);
            int result = checkField(field);
            if(result!=YES){
                return result;
            }
        }
        //字段检查通过,调用下层接口
        TableDao* tableDao = new TableDao(this->databaseName,DB_CREATE_PATH);
        TableEntity table;
        table.setTableName(tableName);
        table.setDBName(databaseName);
        int jieguo = tableDao->createTable(table);
        if(jieguo==0){
            //表创建成功,写入字段信息
            for(int i = 0;i<fields.size();i++){
                int fieldResult = FieldService().insertField(databaseName,tableName,fields.at(i));
                if(fieldResult!=1){
                    return fieldResult;
                }
            }
            return YES;
        }else{
            return jieguo;
        }
    }else{
        //未指明字段，直接建表
        TableDao* tableDao = new TableDao(this->databaseName,DB_CREATE_PATH);
        TableEntity table;
        table.setTableName(tableName);
        table.setDBName(databaseName);
        return tableDao->createTable(table);
    }

    return YES;
}

vector<TableEntity> TableService::getTBListByDBName(QString databaseName){
    TableDao* tableDao = new TableDao(databaseName,DB_CREATE_PATH);
    return tableDao->getTableList();
}

int TableService::DeleteTable(QString tableName){
    TableDao* tableDao = new TableDao(databaseName,DB_CREATE_PATH);
    return tableDao->deleteTable(tableName);
}

int TableService::ModifyTableName(QString oldName,QString newName){
    TableDao* tableDao = new TableDao(databaseName,DB_CREATE_PATH);
    //一些处理

    return tableDao->modifyTableName(oldName,newName);
}
