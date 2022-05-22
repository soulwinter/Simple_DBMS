#ifndef DBDAO_H
#define DBDAO_H
#include<QString>
#include <iostream>
#include "global.h"
#include <vector>
#include <QFile>
#include <QIODevice>
#include <QCoreApplication>
#include <QDebug>
#include "dbentity.h"
#include "fileutil.h"
#include "global.h"
class DBDao
{
private:
    FileUtil* fileUtil;

public:
    DBDao();

    bool createDatabase(DBEntity database);
    bool deleteDatabase(QString databseName);
    bool modifyDatabaseName(QString oldName,QString newName);
    vector<DBEntity> getDatabaseList();
};

#endif // DBDAO_H
