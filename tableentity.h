#ifndef TABLEENTITY_H
#define TABLEENTITY_H
#include <QString>
#include<time.h>
using namespace std;
class TableEntity
{
private:
    int id;
    QString tableName;
    QString DBName;
    int fieldNum = 0;
    int RecordNum = 0;
    time_t creatTime = time(NULL);
    time_t modifyTime = time(NULL);
    
public:
    TableEntity();
    
    QString getTableName() const;
    void setTableName(const QString &value);
    int getId() const;
    void setId(int value);
    QString getDBName() const;
    void setDBName(const QString &value);
    int getFieldNum() const;
    void setFieldNum(int value);
    int getRecordNum() const;
    void setRecordNum(int value);
    time_t getCreatTime() const;
    void setCreatTime(const time_t &value);
    time_t getModifyTime() const;
    void setModifyTime(const time_t &value);
};

#endif // TABLEENTITY_H
