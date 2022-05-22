#ifndef RECORDDAO_H
#define RECORDDAO_H
#include <qstring.h>
#include "recordentity.h"

using namespace std;
class RecordDao
{
public:
    RecordDao();

    /* Path 为表的绝对路径(例如:F:\\DBMS\\user)，tableName为表名 */
    int AddRecord(QString path,QString tableName,RecordEntity r);

    /* 更新表中的旧内容（根据内容的id找到对应的行) */
    int UpdateRecord(QString path,QString tableName,RecordEntity oldContentEntity,RecordEntity newContentEntity);

    /* 删除表中对应的内容(注意，如果一个表中的记录数超过99会产生bug) */
    int deleteRecord(QString path,QString tableName,RecordEntity r);

    //获取所有记录
    vector<RecordEntity> getRecordList(QString dbName,QString tableName);

};

#endif // RECORDDAO_H
