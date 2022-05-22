#include "dbentity.h"

int DBEntity::getId()
{
    return id;
}

void DBEntity::setId(int value)
{
    id = value;
}

QString DBEntity::getName() const
{
    return name;
}

void DBEntity::setName(const QString &value)
{
    name = value;
}

int DBEntity::getType() const
{
    return type;
}

void DBEntity::setType(int value)
{
    type = value;
}

QString DBEntity::getFilePath() const
{
    return filePath;
}

void DBEntity::setFilePath(const QString &value)
{
    filePath = value;
}

int DBEntity::getTableNum() const
{
    return tableNum;
}

void DBEntity::setTableNum(int value)
{
    tableNum = value;
}

time_t DBEntity::getCreateTime() const
{
    return createTime;
}

void DBEntity::setCreateTime(const time_t &value)
{
    createTime = value;
}

DBEntity::DBEntity()
{
    
}
