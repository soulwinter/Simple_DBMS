#ifndef EXPORT_H
#define EXPORT_H
#include <vector>
#include <QString>
#include <map>
#include "global.h"
#include "fileutil.h"
#include "tableentity.h"
#include "tableservice.h"
#include "fielddao.h"
#include "recorddao.h"
#include "sqlanalyze.h"
using namespace std;

class Export
{
public:
    // 导出用法
    // Export(<数据库名>, <系统路径>(比如数据库 abc 的路径为 xxx2/xxx3/abc, 则此处输入 xxx2/xxx3）).createFile();
    //创建路径为 global.h 里的 OUTPUT_PATH
    Export(QString dbName, QString systemPath);
    void createFile();

    // 导入
    Export();
    int importFile(QString path);

private:
    QString dbName;
    QString systemPath;
};

#endif // EXPORT_H
