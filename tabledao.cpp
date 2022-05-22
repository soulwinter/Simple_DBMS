#include "tabledao.h"


TableDao::TableDao(QString dbName, QString systemPath) {
    filePath = systemPath + "//" + dbName + "//";
    this->dbName = dbName;
}
/*
5. 表结构文件
存储字段的信息
文件名：表名.tdf
文件结构：计数器#数据库名#表名（文件第一行）
id#字段名#字段顺序#字段类型#字段长度#最大值#最小值#默认值##是否主键#是否为空#是否唯一#注释
6. 表记录文件
存储每一条记录
文件名：表名.trd
文件结构：计数器#数据库名#表名（文件第一行）
id#属性01值#属性02值#属性03值……..
 *
 */
int TableDao::createTable(TableEntity tableName) {
    // filePath 为系统路径+数据库, tableName 为表名，以后还要改
    QString path = filePath + tableName.getTableName();
    if (FileUtil().ifExistFolder(path)) {
            qDebug() << "错误代码 301: 该表已存在";
            return TABLE_EXISTS;
    }

    // 修改数据库 .db 文件的计数器内容
    int tableNumber = FileUtil().getCounter(filePath + "//" + dbName + ".db");
    FileUtil().replaceContent(filePath + "//" + dbName + ".db", QString::number(tableNumber) + "#" + dbName, QString::number(tableNumber + 1) + "#" + dbName);
    FileUtil().writeAtTail(filePath + "//" + dbName + ".db",
                           QString::number(tableNumber + 1) + "#" +
                           tableName.getTableName() + "#" +
                           QString::number(tableName.getFieldNum()) + "#" +
                           QString::number(tableName.getRecordNum()) + "#" +
                           QString::number(tableName.getCreatTime()) + "#" +
                           QString::number(tableName.getModifyTime()));
    // Id#表名#字段数#记录数#创建时间#上次修改时间



    // 创建表文件夹
    if (FileUtil().createFolder(path)) {

        // 创建 .tdf 文件
        QString tdfFilePath = path + "//" + tableName.getTableName() + ".tdf";
        QString trdFilePath = path + "//" + tableName.getTableName() + ".trd";
        if (FileUtil().createFile(tdfFilePath))
        {
            // 在这里写 tdf 文件内容
            QString tdfFileContent = "";
            tdfFileContent.append("0#" + dbName + "#" + tableName.getTableName());
            FileUtil().writeAllFileForQString(tdfFilePath, tdfFileContent);



        } else {
            qDebug() << "错误代码 303: 创建 .tdf 文件失败";
            return FAILED_CREATE_TABLE_FILE;
        }

        // 创建 .trd 文件
        if (FileUtil().createFile(trdFilePath))
        {
            // 在这里写 trd 文件内容
            QString trdFileContent = "";
            trdFileContent.append("0#" + dbName + "#" + tableName.getTableName());
            FileUtil().writeAllFileForQString(trdFilePath, trdFileContent);
        } else {
            qDebug() << "错误代码 303: 创建 .trd 文件失败";
            return FAILED_CREATE_TABLE_FILE;
        }
        // 文件夹及两个文件都创建成功，返回 0

    } else {
        qDebug() << "错误代码 302: 创建表时创建文件夹失败";
        return FAILED_CREATE_FOLDER_WHEN_CREATE_FOLDER;
    }
    // 文件夹及两个文件都创建成功，返回 0
    return 0;
}

int TableDao::deleteTable(QString tableName)
{
    QString path = filePath + tableName;
    if (FileUtil().ifExistFolder(path)) {
           // bool deleteFolder(QString path);
        if (FileUtil().deleteFolder(path))
        {
            //删除数据库里的记录
            QString dbPath = DB_CREATE_PATH;
            dbPath.append(dbName+"//");
            dbPath.append(dbName+".db");
            int num = FileUtil().readLine(dbPath, 1).left(1).toInt();
            for (int i = 2; i <= num + 1; i++) {
                QString resm = FileUtil().readLine(dbPath, i);
                //QString res = "123#456#0#45";
                QStringList strList = resm.split("#");
                if (strList[1] == tableName) {
                    FileUtil().deleteLine(dbPath, i);
                    break;
                }/**/
            }
            num--;
            FileUtil().updateCounter(dbPath, QString::number(num));
            return 0;
        } else {
            qDebug() << "错误代码 305: 删除表的文件夹失败";
            return FAILED_DELETE_TABLE_FOLDER;
        }
    } else {
        qDebug() << "错误代码 304: 表的文件夹不存在";
        return FOLDER_NOT_EXISTS;
    }
    return 0;
}

//清空xxx.trd文件
int TableDao::clearTableData(QString tableName) {
    QString path = filePath + tableName + "//" + tableName + ".trd";
    if (FileUtil().ifExistFile(path))
    {
        QString firstLine = FileUtil().readLine(path, 1);
        if (FileUtil().deleteFile(path)) {
            if (FileUtil().createFile(path)) {
                FileUtil().writeAllFileForQString(path, firstLine);
            } else {
                qDebug() << "错误代码 306: 修改 TRD 文件失败";
                return FAILED_MODIFY_TRD;
            }
        } else {
            qDebug() << "错误代码 306: 修改 TRD 文件失败";
            return FAILED_MODIFY_TRD;
        }
    } else {
        qDebug() << "错误代码 306: 修改 TRD 文件失败";
        return FAILED_MODIFY_TRD;
    }
    return 0;
}

vector<TableEntity> TableDao::getTableList()
{
    QString path = filePath + "//" + dbName + ".db";
    int number = FileUtil().getCounter(path);
    vector<TableEntity> list;
    for (int i = 2; i <= number + 1; i++)
    {
        qDebug() << i;
        QString lineRead = FileUtil().readLine(path, i);
        for (int j = 0; j < lineRead.length(); j++)
        {
            if (lineRead[j] == '#')
                lineRead[j] = ' ';
        }


        int id;
        QString tableName;
        int fieldNumber;
        int recordNumber;
        int createTime;
        int lastModifiedTime;

        QTextStream strStream(&lineRead);
        strStream >> id >> tableName >> fieldNumber >> recordNumber >> createTime >> lastModifiedTime;

        // sscanf(pathInChar, "%d#%s#%d#%d#%d#%d", id, tableName, fieldNumber, recordNumber, createTime, lastModifiedTime);

        TableEntity newTable;
        newTable.setId(id);
        newTable.setDBName(dbName);
        newTable.setTableName(tableName);
        newTable.setFieldNum(fieldNumber);
        newTable.setRecordNum(recordNumber);
        newTable.setCreatTime((time_t)createTime);
        newTable.setModifyTime((time_t)lastModifiedTime);
        // newTable.setModifyTim
        list.push_back(newTable);
        // Id#表名#字段数#记录数#创建时间#上次修改时间

        // 先一行一行读取，然后转为 char 然后 scanf
    }
    return list;
}

int TableDao::modifyTableName(QString oldName, QString newName) {
    QString path = filePath + oldName;
    if(FileUtil().ifExistFolder(filePath+newName)){
        qDebug() << "错误代码 307-4: 重名导致重命名失败";
        return FAILED_RENAME;
    }

    FileUtil().replaceContent(filePath + "//" + dbName + ".db",
                              "#" + oldName + "#",
                              "#" + newName + "#");


    if (FileUtil().ifExistFolder(path)) {
        QString tdfFilePath = path + "//" + oldName + ".tdf";
        QString trdFilePath = path + "//" + oldName + ".trd";
        FileUtil().replaceContent(tdfFilePath, oldName, newName);
        FileUtil().replaceContent(trdFilePath, oldName, newName);
        QFile fileTDF(tdfFilePath);
        if (fileTDF.rename(path + "//" + newName + ".tdf"))
        {

        } else {
            qDebug() << "错误代码 307-1: 重命名失败";
            return FAILED_RENAME;
        }
        QFile fileTRD(trdFilePath);
        if (fileTRD.rename(path + "//" + newName + ".trd"))
        {

        } else {
            qDebug() << "错误代码 307-2: 重命名失败";
            return FAILED_RENAME;
        }
        QDir dir(path);
        if (dir.rename(path, filePath + newName))
        {

        } else {
            qDebug() << "错误代码 307-3: 重命名失败";
            return FAILED_RENAME;
        }


    } else {
        qDebug() << "错误代码 304: 表的文件夹不存在";
        return FOLDER_NOT_EXISTS;
    }
    return 0;
}

