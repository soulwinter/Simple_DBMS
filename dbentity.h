#ifndef DBENTITY_H
#define DBENTITY_H
#include <QString>
#include <time.h>
using namespace std;
class DBEntity
{
private:
    int id;
    QString name;
    int type;
    QString filePath;
    int tableNum;
    time_t createTime;

public:
    DBEntity();
    
    int getId();
    void setId(int value);
    QString getName() const;
    void setName(const QString &value);
    int getType() const;
    void setType(int value);
    QString getFilePath() const;
    void setFilePath(const QString &value);
    int getTableNum() const;
    void setTableNum(int value);
    time_t getCreateTime() const;
    void setCreateTime(const time_t &value);
};

#endif // DBENTITY_H
