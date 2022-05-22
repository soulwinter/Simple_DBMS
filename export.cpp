#include "export.h"
#include "fileutil.h"

Export::Export(QString dbName, QString systemPath)
{
    this->dbName = dbName;
    this->systemPath = systemPath;
}

Export::Export()
{

}

int Export::importFile(QString path)
{
    vector<QString> sentences;
    if (FileUtil().ifExistFile(path))
    {

         QString content = FileUtil().readAll(path);
         QStringList localSplit = content.split("\n");
         sqlAnalyze* sqlnalyze = NULL;
         QString dbName = "";
         for(int i=0;i<localSplit.size();i++){
             QString sql = localSplit.at(i);
             if(sql.contains("database")){
                 sqlnalyze = new sqlAnalyze(sql.split(' ').at(2));
                 dbName = sql.split(' ').at(2);
                 int result = sqlnalyze->createDatebaseSql(sql);
                 if(!result){
                     return result;
                 }
             }else if(sql.contains("create")){
                 int result = sqlnalyze->createTableSql(sql,dbName);
             }else if(sql.contains("alter")){
                 sqlnalyze->operatorInAlterTable(sql);
             }else if(sql.contains("insert")){
                 QString path = DB_CREATE_PATH;
                 path.append(dbName);
                 sqlnalyze->analysql(path,sql);
             }
         }
         return true;


    }
    // 如果文件不存在则直接返回
    return false;
}

void Export::createFile()
{
    // 新建文件
    QString path = OUTPUT_PATH;
    path.append(dbName).append(".sql");
    FileUtil().deleteFile(path);
    FileUtil().createFile(path);


    // 创建数据库
    FileUtil().writeAtTail(path, "create database " + dbName);

    vector<TableEntity> tBList = TableService().getTBListByDBName(dbName); // 获取表实体 vector

    for (int i = 0; i < tBList.size(); i++)
    {
        QString tableName = tBList[i].getTableName();

        // 获取 fieldEntity
        vector<FieldEntity> fields = FieldDao(dbName, systemPath, tableName).getFieldList(tableName);



        // 以下 for 循环用于创建表（包含 default, 但不包含 primary 等特性）
        QString createTableSentence = "\ncreate table " + tableName + "("; // create table 语句写好前半部分
        for (int eachField = 0; eachField < fields.size(); eachField++)
        {
            QString fieldName = fields[eachField].getName();
            int fieldType = fields[eachField].getType();
            QString fieldTypeString = "";
            QString defaultTypeString = "";
            // 获取类型
            if (fieldType == 1) {
                fieldTypeString = "int";
            }
            if (fieldType == 2) {
                fieldTypeString = "bool";
            }
            if (fieldType == 3) {
                fieldTypeString = "time";
            }
            if (fieldType == 4) {
                fieldTypeString = "varchar(" + QString::number(fields[eachField].getLength()) + ")";
            }
            if (fieldType == 5) {
                fieldTypeString = "double";
            }

            if (fields[eachField].getDefaultValue() != "null")
            {
                if (fieldType == 4 or fieldType == 3)
                {
                    defaultTypeString = " default '" + fields[eachField].getDefaultValue() + "'";
                } else {
                    defaultTypeString = " default " + fields[eachField].getDefaultValue();
                }

            }


            createTableSentence += fieldName + " " + fieldTypeString + defaultTypeString;
            if (eachField != fields.size() - 1)
            {
                createTableSentence += ", ";
            }
            // 如果不是最后一个就加一个逗号
        }
        createTableSentence += ");";
        FileUtil().writeAtTail(path, createTableSentence);

        //  以下 for 循环用于添加 primary key 等
        for (int eachField = 0; eachField < fields.size(); eachField++)
        {

            int fieldType = fields[eachField].getType();
            QString fieldTypeString = "";
            // 获取类型
            if (fieldType == 1) {
                fieldTypeString = "int";
            }
            if (fieldType == 2) {
                fieldTypeString = "bool";
            }
            if (fieldType == 3) {
                fieldTypeString = "time";
            }
            if (fieldType == 4) {
                fieldTypeString = "varchar(" + QString::number(fields[eachField].getLength()) + ")";
            }
            if (fieldType == 5) {
                fieldTypeString = "double";
            }


            if (fields[eachField].getIsPK())
            {
                QString alterTableString = "alter table " + tableName + " modify " + fields[eachField].getName() + " " + fieldTypeString + " primary key";
                FileUtil().writeAtTail(path,alterTableString);

            } else if (fields[eachField].getIsUnique())
            {
                QString alterTableString = "alter table " + tableName + " modify " + fields[eachField].getName() + " " + fieldTypeString + " unique";
                FileUtil().writeAtTail(path,alterTableString);
            } else if (fields[eachField].getIsNotNUll())
            {
                QString alterTableString = "alter table " + tableName + " modify " + fields[eachField].getName() + " " + fieldTypeString + " not null";
                FileUtil().writeAtTail(path,alterTableString);
            }

        }

        // 获取 records
        vector<RecordEntity> records = RecordDao().getRecordList(dbName, tableName);
        for (int eachRecord = 0; eachRecord < records.size(); eachRecord++)
        {
            map<QString, QString> recordMap = records[eachRecord].getValueMap();
            vector<QString> fieldNames;
            vector<QString> fieldValues;


            for(map<QString, QString>::iterator iter = recordMap.begin(); iter != recordMap.end(); ++iter)
            {
                fieldNames.push_back(iter->first);
                fieldValues.push_back(iter->second);

            }
            QString insertSentence = "insert into " + tableName + " (";
            for (int fn = 0; fn < fieldNames.size(); fn++)
            {
                insertSentence += fieldNames[fn];
                if (fn != fieldNames.size() - 1)
                {
                    insertSentence += ",";
                }
            }
            insertSentence += ") values (";
            for (int fn = 0; fn < fieldValues.size(); fn++)
            {
                insertSentence += fieldValues[fn];
                if (fn != fieldValues.size() - 1)
                {
                    insertSentence += ",";
                }
            }
            insertSentence += ")";
            FileUtil().writeAtTail(path, insertSentence);
            //FileUtil().writeAtTail(path, "insert into " + tableName + " (" + iter->first + ") values (" + iter->second + ");");

        }


    }













}
