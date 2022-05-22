#include "dbservice.h"

DBService::DBService()
{
    dbDao = new DBDao();
    fileUtil = new FileUtil();
}

//创建数据库方法
bool DBService::createDatabase(QString databaseName){
    //判断数据库名称是否超过128个字符
    if(databaseName.length()>128){
        return false;
    }
    //检查数据库名是否为空
    if(databaseName.length()==0){
        return false;
    }
    //判断数据库名是否包含空格
    if(!fileUtil->ifContainsSpace(databaseName)){
        return false;
    }
    //业务逻辑检查通过,初始化数据库实体
    vector<DBEntity> databaseList = dbDao->getDatabaseList();
    DBEntity dbEntity;
    if(databaseList.size()==0){
        dbEntity.setId(1);
    }else{
        dbEntity.setId(databaseList.at(databaseList.size()-1).getId()+1);
    }
    dbEntity.setCreateTime(time(NULL));
    dbEntity.setFilePath(DB_CREATE_PATH + databaseName);
    dbEntity.setName(databaseName);
    dbEntity.setTableNum(0);
    dbEntity.setType(DB_USER);
    //调用下一层的接口
    return dbDao->createDatabase(dbEntity);
}

bool DBService::modifyDatabaseName(QString oldName,QString newName){
    return dbDao->modifyDatabaseName(oldName,newName);
}

bool DBService::deleteDatabase(QString databaseName){
    return dbDao->deleteDatabase(databaseName);
}

int DBService::countDBNum(){
    return dbDao->getDatabaseList().size();
}

vector<DBEntity> DBService::getDBList(){
    return dbDao->getDatabaseList();
}

