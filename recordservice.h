#pragma once
#include<QString>
#include <iostream>
#include <vector>
#include "fileutil.h"
#include "global.h"
#include "recordentity.h"
#include "fieldentity.h"
#include "recorddao.h"
class RecordService {
private:

public:
    RecordService();
    //读取文件查看约束
    int insertRecord(QString dbName, QString tableName, RecordEntity recordEntity);
//    //deleteInput举例：delete from teacher where tid = 1;
//    int deleteRecord(QString deleteInput, QString dbName); //需要完成解析删除sql语句的任务
//    //同上
//    int modifyRecord(QString updateInput, QString dbName);
    int deletrecord(RecordEntity recordEntity, QString dbName,QString tableName);
    int modifyrecord(QString dbName, QString tableName, RecordEntity old, RecordEntity ne);
    vector<RecordEntity> getRecordList(QString dbName,QString tableName);
};
