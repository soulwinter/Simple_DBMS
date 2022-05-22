#include "recorddao.h"
#include "fileutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <qlist.h>
#include <qfile.h>
#include <qtextstream.h>
#include "fieldservice.h"

RecordDao::RecordDao()
{

}

int RecordDao::AddRecord(QString path,QString tableName,RecordEntity r)
{
    QFile destinationFile;// 表记录文件
    FILE* sourceFile;// 表结构文件
    QList<QString> fieldList;// 存取字段名的链表
    int id = r.getId();

    /* 得到表结构文件的绝对路径 */
    QString sourceFilePath = path;
    sourceFilePath.append("//"+tableName);
    sourceFilePath.append("//").append(tableName).append(".tdf");

    /* 将表结构文件的路径转为char*类型 */
    char* pathCharSource;
    QByteArray b = sourceFilePath.toLatin1();
    pathCharSource = b.data();

    /* 打开表结构文件 */
    if ((sourceFile = fopen(pathCharSource , "r")) == NULL)
    {
        fprintf(stderr , "Error in opening file: %s\n" , pathCharSource);
        return 0;
    }

    /* 读取到文件的第二行 */
    for (char c ; (c = getc(sourceFile)) != '\n';);

    /* 读取表结构文件中的字段名 */
    while (feof(sourceFile) == 0)
    {
        /* 读取id */
        int i;
        fscanf(sourceFile , "%d" , &i);

        /* 读取字段名 */
        char fieldNameChar[20];
        char c;
        int k;
        c = getc(sourceFile);// 清除#号
        for (k = 0 ;feof(sourceFile) == 0 && (c = getc(sourceFile)) != '#' && c != '\n' ; k ++)
            fieldNameChar[k] = c;
        fieldNameChar[k] = '\0';// 在字段名最后加上

        /* 将读取的字段名转化为QString类 */
        QString fieldName = QString::fromUtf8(fieldNameChar);

        /* 将该字段名存到链表中 */
        fieldList.append(fieldName);

        /* 跳转到下一行或是结束读取 */
        while ((c = getc(sourceFile)) != '\n')
            if (feof(sourceFile) != 0)
                break;
    }

    /* 关闭表结构文件 */
    fclose(sourceFile);


    /* 以追写模式打开表记录文件 */
    QString destinationFilePath = path;
//    sourceFilePath.append("//"+tableName);
    destinationFilePath.append("//"+tableName);
    destinationFilePath.append("//").append(tableName).append(".trd");
    destinationFile.setFileName(destinationFilePath);

    //写之前先读取一下最后一条记录的id
    QString num = FileUtil().readLine(destinationFilePath,1).left(1);//获取记录数
    QString recordStr = FileUtil().readLine(destinationFilePath,num.toInt()+1);
    QStringList localSplit = recordStr.split('#');
    id = localSplit[0].toInt()+1;

    destinationFile.open(QIODevice::Append);
    QTextStream stream(&destinationFile);// 打开文件流

    /* 写入id */
    stream << '\n';
    stream << id;

    /* 写入各字段名的值 */
    for (int i = 0 ; i < fieldList.size() ; i++)
    {
        stream << '#';
        QString field = fieldList.at(i);
        QString value = r.getValue(field);
        stream << value;
    }

    /* 关闭文件 */
    destinationFile.close();

    //更新计数器
    int newNum = num.toInt()+1;
    FileUtil().updateCounter(destinationFilePath,QString::number(newNum));

    return 1;
}

int RecordDao::UpdateRecord(QString path,QString tableName,RecordEntity oldContentEntity,RecordEntity newContentEntity)
{
    QList<QString> fieldList;// 按顺序存取字段名的链表
    QString oldContent = "";// 存储旧的内容
    QString newContent = "";// 存储新的内容

    /* 获取表结构文件的路径 */
    FILE* sourceFile;// 表结构文件

    /* 得到表结构文件的绝对路径 */
    QString sourceFilePath = DB_CREATE_PATH;
    sourceFilePath.append(path + "//" + tableName);
    sourceFilePath.append("//").append(tableName).append(".tdf");

    /* 将表结构文件的路径转为char*类型 */
    char* pathCharSource;
    QByteArray a = sourceFilePath.toLatin1();
    pathCharSource = a.data();

    /* 打开表结构文件 */
    if ((sourceFile = fopen(pathCharSource , "r")) == NULL)
    {
        fprintf(stderr , "Error in opening file: %s\n" , pathCharSource);
        return 0;
    }

    /* 读取到文件的第二行 */
    for (char c ; (c = getc(sourceFile)) != '\n';);

    /* 读取表结构文件中的字段名 */
    while (feof(sourceFile) == 0)
    {
        /* 读取id */
        int i;
        fscanf(sourceFile , "%d" , &i);

        /* 读取字段名 */
        char fieldNameChar[20];
        char c;
        int k;
        c = getc(sourceFile);// 清除#号
        for (k = 0 ;feof(sourceFile) == 0 && (c = getc(sourceFile)) != '#' && c != '\n' ; k ++)
            fieldNameChar[k] = c;
        fieldNameChar[k] = '\0';// 在字段名最后加上

        /* 将读取的字段名转化为QString类 */
        QString fieldName = QString::fromUtf8(fieldNameChar);

        /* 将该字段名存到链表中 */
        fieldList.append(fieldName);

        /* 跳转到下一行或是结束读取 */
        while ((c = getc(sourceFile)) != '\n')
            if (feof(sourceFile) != 0)
                break;
    }

    /* 关闭表结构文件 */
    fclose(sourceFile);

    /* 添加旧与新的内容 */
    /* 添加id */
    oldContent.append(QString::number(oldContentEntity.getId()));
    newContent.append(QString::number(oldContentEntity.getId()));

    /* 添加字段内容 */
    for (int i = 0 ; i < fieldList.size() ; i++)
    {
        /* 旧内容的添加 */
        oldContent.append('#').append(oldContentEntity.getValue(fieldList.at(i)));

        /* 新内容的添加 */
        newContent.append('#').append(newContentEntity.getValue(fieldList.at(i)));
    }

    /* 获取表记录文件的绝对路径 */
    QString recordFilePath = DB_CREATE_PATH;
    recordFilePath.append(path + "//" +tableName + "//" + tableName + ".trd");

    /* 调用fileutil中的replaceContent来替换文件的内容 */
    if (FileUtil().replaceContent(recordFilePath , oldContent , newContent))
        return 1;
    else
        return 0;
}

int RecordDao::deleteRecord(QString path,QString tableName,RecordEntity r)
{
    int id = r.getId();// 该记录的id号
    QString recordFilePath = path.append("//").append(tableName).append(".trd");// 记录文件的路径qstring
    char* recordFilePathChar;// 记录文件的路径char*
    FILE* recordFile;// 记录文件
    FILE* tempFile;// 临时文件
    int row = id + 1;// 记录所需要删除的记录是文件的第几行

    /* 删除对应记录的行 */
    FileUtil().deleteLine(recordFilePath , row);

    /* 更新其余记录的id */

    /* 获取记录文件的路径char* */
    QByteArray a = recordFilePath.toLatin1();
    recordFilePathChar = a.data();

    /* 打开记录文件 */
    if ((recordFile = fopen(recordFilePathChar , "r")) == NULL)
    {
        fprintf(stderr , "Error in opening file: %s\n" , recordFilePathChar);
        return 0;
    }

    /* 打开临时文件 */
    if ((tempFile = fopen("C://aaa.txt" , "w")) == NULL)
    {
        fprintf(stderr , "Error in opening temp file.\n");
        return 0;
    }

    /* 读取到原先删除的行的位置开始 */
    for (int i = 0 ; i < id ; ++i)
    {
        char str[200];
        fgets(str , 200 , recordFile);
        fputs(str , tempFile);
    }

    /* 开始修改余下记录的id */
    while (feof(recordFile) == 0)
    {
        int tempId;
        char tempIdChar[10];
        int h;// 存储百位
        int d;// 存储十位
        int u;// 存储个位

        /* 首先读取第一个id，但不存入临时文件，而是存入id+1 */
        fscanf(recordFile , "%d" , &tempId);
        tempId--;
        h = tempId / 100;
        d = (tempId / 10) % 10;
        u = tempId % 10;
        if (h == 0)
            if (d == 0)
            {
                tempIdChar[0] = '0' + u;
                tempIdChar[1] = '\0';
            }
            else
            {
                tempIdChar[0] = '0' + d;
                tempIdChar[1] = '0' + u;
                tempIdChar[2] = '\0';
            }
        else
        {
            tempIdChar[0] = '0' + h;
            tempIdChar[1] = '0' + d;
            tempIdChar[2] = '0' + u;
            tempIdChar[3] = '\0';
        }


        /* 写入id */
        fprintf(tempFile , "%s" , tempIdChar);

        /* 其他数据照常往下写 */
        char str[200];
        fgets(str , 200 , recordFile);
        fputs(str , tempFile);
    }

    /* 关闭两个文件流 */
    fclose(tempFile);
    fclose(recordFile);

    /* 打开两个文件 */
    if ((recordFile = fopen(recordFilePathChar , "w")) == NULL)
    {
        fprintf(stderr , "Error in opening file: %s\n" , recordFilePathChar);
        return 0;
    }
    if ((tempFile = fopen("C://aaa.txt" , "r")) == NULL)
    {
        fprintf(stderr , "Error in opening temp file.\n");
        return 0;
    }

    /* 将临时文件里的内容写到记录文件中 */
    while (feof(tempFile) == 0)
    {
        char str[500];
        fgets(str , 500 , tempFile);
        fputs(str , recordFile);
    }

    /* 关闭文件，删除临时文件 */
    fclose(tempFile);
    fclose(recordFile);

    remove("C://aaa.txt");

    //更新计数器
    int counter = FileUtil().getCounter(recordFilePath);
    FileUtil().updateCounter(recordFilePath,QString::number(counter-1));


    return 1;
}

//读取表中的所有记录
vector<RecordEntity> RecordDao::getRecordList(QString dbName, QString tableName){
    //设置路径
    QString path = DB_CREATE_PATH;
    path.append(dbName+"//"+tableName+"//"+tableName+".trd");

    /*获取记录*/
    vector<RecordEntity> recordList;
    //先读取字段信息
    vector<FieldEntity> fieldList = FieldService().getFieldList(dbName,tableName);
    //首先读取到有多少条记录
    int num = FileUtil().getCounter(path);
    //循环读取并设置到RecordEntity实体中
    for(int i=2;i<=num + 1;i++){

        QString recordStr = FileUtil().readLine(path,i);
        QStringList recordStrList = recordStr.split('#');

        RecordEntity record;
        record.setId(recordStrList[0].toInt());
        //根据顺序，结合字段填充map
        for(int j = 1;j<recordStrList.length();j++){
            record.SetValue(fieldList.at(j-1).getName(),recordStrList[j]);
        }
        //填充完成加入列表
        recordList.push_back(record);
    }

    return recordList;
}
