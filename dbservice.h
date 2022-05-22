#ifndef DBSERVICE_H
#define DBSERVICE_H
#include <QString>
#include <vector>
#include "dbentity.h"
#include "dbdao.h"
#include "global.h"
#include "fileutil.h"

class DBService
{
public:
    DBService();
    bool createDatabase(QString databaseName);
    bool modifyDatabaseName(QString oldName,QString newName);
    bool deleteDatabase(QString databaseName);
    int countDBNum();
    vector<DBEntity> getDBList();
private:
    DBDao* dbDao;
    FileUtil * fileUtil;
};

#endif // DBSERVICE_H
