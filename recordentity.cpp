#include "recordentity.h"
#include<map>
int RecordEntity::getId() const
{
    return id;
}

map<QString, QString> RecordEntity::getValueMap() const
{
    return valueMap;
}

QString RecordEntity::getValue(QString field){
    if(valueMap.count(field)>0){
          return valueMap[field];
    }
    return "不存在字段";   
}

void RecordEntity::setId(int value)
{
    id = value;
}

void RecordEntity::SetValue(QString field,QString value){
    valueMap[field] = value;
}

RecordEntity::RecordEntity()
{
    
}
