#include "tableentity.h"

QString TableEntity::getTableName() const
{
    return tableName;
}

void TableEntity::setTableName(const QString &value)
{
    tableName = value;
}

int TableEntity::getId() const
{
    return id;
}

void TableEntity::setId(int value)
{
    id = value;
}

QString TableEntity::getDBName() const
{
    return DBName;
}

void TableEntity::setDBName(const QString &value)
{
    DBName = value;
}

int TableEntity::getFieldNum() const
{
    return fieldNum;
}

void TableEntity::setFieldNum(int value)
{
    fieldNum = value;
}

int TableEntity::getRecordNum() const
{
    return RecordNum;
}

void TableEntity::setRecordNum(int value)
{
    RecordNum = value;
}

time_t TableEntity::getCreatTime() const
{
    return creatTime;
}

void TableEntity::setCreatTime(const time_t &value)
{
    creatTime = value;
}

time_t TableEntity::getModifyTime() const
{
    return modifyTime;
}

void TableEntity::setModifyTime(const time_t &value)
{
    modifyTime = value;
}

TableEntity::TableEntity()
{
    
}
