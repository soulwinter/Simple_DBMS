#ifndef FIELDDAO_H
#define FIELDDAO_H
#include <QString>
#include <vector>
#include "fieldentity.h"
#include "fileutil.h"
#include <qdebug.h>
#include <QStringList>
using namespace std;
class FieldDao
{
private:
    QString filePath;
    FileUtil fileUtil;
public:
    FieldDao(QString dbName, QString systemPath, QString tableName);
    int insertField(QString dbName, QString tableName, FieldEntity fieldEntity);
    vector<FieldEntity> getFieldList( QString tableName);
    int deleteField(QString dbName, QString tableName, QString fieldName); //这个方法注意还需删除表记录中的对应值
    int modifyField(QString tableName,QString oldFieldName, FieldEntity newfieldEntity);
};

#endif // FIELDDAO_H
