#ifndef FILEUTIL_H
#define FILEUTIL_H
#include <QString>
#include <qdir.h>
#include<qdatastream.h>
#include<qtextstream.h>
#include<vector>
#include"global.h"
using namespace std;

class FileUtil
{
public:
    FileUtil();
    //4.13
    //创建文件夹
    bool createFolder(QString path);
    //修改文件夹名
    bool modifyFolderName(QString path, QString newName);
    //删除文件夹
    bool deleteFolder(QString path);
    //创建文件
    bool createFile(QString path);
    //删除文件
    bool deleteFile(QString path);
    //写入整个文件
    bool writeAllFile(QString path, char* content);
    bool writeAllFileForQString(QString path, QString content);
    //判断是否存在文件
    bool ifExistFile(QString path);
    //判断是否存在文件夹
    bool ifExistFolder(QString path);
    //判断数据库名是否包含空格
    bool ifContainsSpace(QString databaseName);
    //4.14
    //读一行----------
    QString readLine(QString path, int line);
    //往文件末尾写入一条数据-------
    void writeAtTail(QString path,QString content);
    //读取整个文件-----------
    QString readAll(QString path);
    //写入多条记录--------
    bool writeManyAtTail(QString path,vector<QString> content);
    //替换文件某处内容-----------
    bool replaceContent(QString path,QString oldContent,QString newContent);
    //删除某一行--------
    void deleteLine(QString path,int line);
    //修改计数器(慎用，仅适合文件第一行只有数字)
    void updateCounter(QString path,QString num);
    //检查字段名是否合法
    int checkFieldName(QString fieldName);
    //获取计数器值
    int getCounter(QString path);
    //得到字段名对应的ID
    int getLineByName(QString path, QString Name);

};
#endif // FILEUTIL_H
