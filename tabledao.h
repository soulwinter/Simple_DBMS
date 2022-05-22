#ifndef TABLEDAO_H
#define TABLEDAO_H
#include <QString>
#include <QDebug>
#include <stdio.h>
#include "global.h"
#include "fileutil.h"
#include "tableentity.h"
using namespace std;



class TableDao
{
private:
    QString filePath;
    QString dbName; //数据库名
    FileUtil fileUtil;
public:
    TableDao(QString dbName, QString systemPath);
    // 请注意，systemPath 为系统路径，至数据库的上一层，例如：”/Users/soulwinter/Desktop/DBMS/" 最后一个斜杠可以加也可以不加
    // dbName 为表所在的数据库名称

    int createTable(TableEntity tableName);
    // 请注意，tableName 应尽可能包含 TableEntity 包含的所有属性，全不为空。

    int deleteTable(QString tableName);
    vector<TableEntity> getTableList();
    int modifyTableName(QString oldName, QString newName);
    int clearTableData(QString tableName); //清空xxx.trd文件

};

#endif // TABLEDAO_H
