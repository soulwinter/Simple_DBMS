#ifndef TABLESERVICE_H
#define TABLESERVICE_H
#include <QString>
#include "fieldservice.h"
#include "fieldentity.h"
#include "recordservice.h"
#include "recordentity.h"
#include <vector>
#include "global.h"
#include "fileutil.h"
#include "tableentity.h"
#include "tabledao.h"

using namespace std;
class TableService
{
private:
    QString databaseName; //getset方法下面就省略不写了
    FieldService fieldService;
    RecordService recordService;

    //辅助方法
    int checkField(FieldEntity field);

public:
    TableService();
    int CreateTable(QString tableName,vector<FieldEntity>fields); //根据表名，和字段来创建表
    int DeleteTable(QString tableName);
    int ModifyTableName(QString oldName,QString newName);
    int addField(QString tableName,FieldEntity fieldEntity);
    int insertRecord(QString tableName,RecordEntity record);
    QString getDatabaseName() const;
    void setDatabaseName(const QString &value);
    vector<TableEntity> getTBListByDBName(QString databaseName);
};

#endif // TABLESERVICE_H
