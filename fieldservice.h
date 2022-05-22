#ifndef FIELDSERVICE_H
#define FIELDSERVICE_H
#include <QString>
#include "fieldentity.h"
#include "fielddao.h"

class FieldService
{


public:
    FieldService();
    int insertField(QString dbName,QString tableName,FieldEntity fieldEntity);
    int deleteField(QString dbName,QString tableName,QString fieldName); //这个方法注意还需删除表记录中的对应值
    int modifyField(QString dbName,QString tableName,QString oldFieldName,FieldEntity newfieldEntity);
    vector<FieldEntity> getFieldList(QString dbName,QString tableName);
};

#endif // FIELDSERVICE_H
