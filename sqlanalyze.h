#ifndef SQLANALYZE_H
#define SQLANALYZE_H

#include<QString>
#include <iostream>
#include <vector>
#include<qdatastream.h>
#include<qtextstream.h>
#include <QRegularExpression>
#include "recordentity.h"
#include "fieldentity.h"
#include "fileutil.h"
#include <string>
#include <regex>
#include "fieldservice.h"
#include "recordservice.h"
#include "dbservice.h"
#include "tableservice.h"
#include <set>

typedef struct Groups{ // select field,f(grade) from table where ... Group By field
    double sum;    // 总和
    int num;  // 个数
    double max;
    double min;
}GroupSql;

class sqlAnalyze
{
public:
    sqlAnalyze(QString databaseName);
    int checkInput(QString input);

    //
    bool checkUpdate(QString input);
    int operatorInUpdate(QString* value,QString operater,int type);
    int operatorInDelete(QString input);
    int createTableSql(QString content, QString dbName);
    int createDatebaseSql(QString content);
    int analysql(QString dbpath,QString s);
    int operatorInAlterTable(QString input);
    //void selectSql(QString dbpath, vector<QString> sql);
    int insertSql(QString dbpath, vector<QString> sql);
    vector<RecordEntity> selectSql(QString dbName, QString input);
    
    vector<RecordEntity> selectSqlWithJoin(QString dbName, QString input);
    vector<RecordEntity> dealResult(vector<QString>needView,vector<RecordEntity>result,int size);
    vector<RecordEntity> dealResultWithJuzu(vector<QString>needView,vector<RecordEntity>result,int size);
    vector<RecordEntity> selectSqlsum(QString dbname,QString input);
    vector<RecordEntity> selectGroup(QString group,map<QString,QString> m,vector<RecordEntity> r);


private:
    QString databaseName;
    bool ifSatisfyCondition(RecordEntity record,vector<QString> condition);

};

#endif // SQLANALYZE_H
