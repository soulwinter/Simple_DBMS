#include "dbdao.h"

DBDao::DBDao()
{
    this->fileUtil = new FileUtil();
}

bool DBDao::createDatabase(DBEntity databaseName) {
    if (FileUtil().ifExistFolder(databaseName.getFilePath())) {
        return false;
    }
    if (FileUtil().createFolder(databaseName.getFilePath())) {
        if (FileUtil().createFile(databaseName.getFilePath() + "//" + databaseName.getName() + ".db")) {
            //如果是系统数据库
            if (databaseName.getType()==1) {
                QString resu = "";
                resu.append(DB_CREATE_PATH);
                resu.append("//System.db");
                QString str;
                str = "1";
                FileUtil().writeAllFileForQString(resu, str);
                //写在文件最后，新的数据库的基本信息
                QString s = "";
                s.append(QString::number(databaseName.getId()));
                s.append("#" + databaseName.getName());
                s.append("#1");
                s.append("#" + QString::number(databaseName.getTableNum()));
                s.append("#" + QString::number(databaseName.getCreateTime()));
                // 写在system.db文件最后
                FileUtil().writeAtTail(resu, s);
                return true;
            }
            //用户创建的数据库
            else {
                // 先更新第一行的计数器
                QString res = FileUtil().readLine("/Users/soulwinter/Desktop/DBMSData/system.db",1);
                int re = res.toInt();
                re += 1;
                FileUtil().updateCounter("/Users/soulwinter/Desktop/DBMSData/system.db", QString::number(re));
                //写在文件最后，新的数据库的基本信息
                QString s = "";
                int id = databaseName.getId();
                s.append("" + QString::number(id));
                s.append('#' + databaseName.getName());
                if (databaseName.getType()==1) s.append("#1");
                else s.append("#2");
                s.append("#" + QString::number( databaseName.getTableNum()));
                s.append("#" + QString::number(databaseName.getCreateTime()));

                // 写在system.db文件最后
                QString path = DB_CREATE_PATH ;
                path = path + "system.db";
                QFile* file = new QFile(path);
                if (file->open(QIODevice::ReadWrite | QIODevice::Append)) {
                    QTextStream txtOutput(file);
                    txtOutput << Qt::endl;
                    txtOutput << s;
                }
                file->close();
                //写自己的.db文件
                QString content = "0#";
                content.append(databaseName.getName());
                FileUtil().writeAllFileForQString(databaseName.getFilePath() + "//" + databaseName.getName() + ".db",content);
            }
            return true;
        }
        return false;
    }
    return false;
}

bool DBDao::deleteDatabase(QString databseName) {
    QString path = DB_CREATE_PATH;
    path.append(databseName);
    if (FileUtil().ifExistFolder(path)) {
        if (FileUtil().deleteFolder(path)) {
            QString res = "";
            res.append(DB_CREATE_PATH);
            res.append("//System.db");
            int num = FileUtil().readLine(res, 1).toInt();
            for (int i = 2; i <= num + 1; i++) {
                QString resm = FileUtil().readLine(res, i);
                //QString res = "123#456#0#45";
                QStringList strList = resm.split("#");
                if (strList[1] == databseName) {
                    FileUtil().deleteLine(res, i);
                    break;
                }/**/
            }
            num--;
            FileUtil().updateCounter(res, QString::number(num));
            return true;
        }
    }
    return false;
}

bool DBDao::modifyDatabaseName(QString oldName, QString newName)
{
    QString rootPath = DB_CREATE_PATH;// 根目录
    QString sysdbPath = rootPath;// 系统数据库路径
    sysdbPath.append("//system.db");
    QString dbPath = rootPath;// 数据库路径
    dbPath.append("//" + oldName + "//" + oldName + ".db");
    QString dbNewPath = rootPath;
    dbNewPath.append("//"+newName+"//"+newName+".db");
    QString dbFolderPath = rootPath;// 数据库文件夹路径
    dbFolderPath.append(oldName);
    QString dbFolderPathNew = rootPath;
    dbFolderPathNew.append(newName);
    QList<QString> tablesNameList;// 存储该数据库所有表名的列表
    QList<QString> tablesRecordPathList;// 存储该数据库所有表记录文件的路径
    QList<QString> tablesStructurePathList;// 存储该数据库所有表结构文件的路径
    FILE* dbFile;// 数据库文件

    QString sameDB= rootPath;
    sameDB.append("//"+newName);
    if(FileUtil().ifExistFolder(sameDB))
       return 0;

    /* 获得数据库路径的char* */
    char* dbPathChar;
    QByteArray a = dbPath.toLatin1();
    dbPathChar = a.data();

    /* 打开数据库 */
    if ((dbFile = fopen(dbPathChar , "r")) == NULL)
    {
        fprintf(stderr , "Error in opening file: %s\n" , dbPathChar);
        return 0;
    }

    /* 读取到数据库第二行 或 没有第二行直接关闭 */
    for (char c ; feof(dbFile) == 0 && (c = getc(dbFile)) != '\n' ;);

    /* 读取数据库中的表名 */
    while (feof(dbFile) == 0)
    {
        int id;// 读取id
        char c;// 处理一行的多余字符
        char tableName[50];// 存储表名
        int index;// 表名的索引

        /* 读取id和#号 */
        fscanf(dbFile , "%d" , &id);
        c = getc(dbFile);

        /* 读取表名 */
        for (index = 0 ; (c = getc(dbFile)) != '#' ; index++)
            tableName[index] = c;
        tableName[index] = '\0';

        /* 将表名存储到链表中 */
        tablesNameList.append(QString::fromUtf8(tableName));

        /* 清理一行中的余下字符 */
        while (feof(dbFile) == 0 && (c = getc(dbFile)) != '\n');
    }

    /* 关闭数据库文件 */
    fclose(dbFile);

    /* 根据表名链表获得所有所有表的记录文件和结构文件的路径 */
    for (int number = 0 ; number < tablesNameList.size() ; number++)
    {
        QString tableName = tablesNameList.at(number);// 表名
        QString tableRecordPath = rootPath;// 表记录路径
        tableRecordPath.append(oldName + "//" + tableName + "//" + tableName + ".trd");
        QString tableStructurePath = rootPath;// 表结构路径
        tableStructurePath.append(oldName + "//" + tableName + "//" + tableName + ".tdf");

        tablesRecordPathList.append(tableRecordPath);
        tablesStructurePathList.append(tableStructurePath);
    }

    /* 更改所有表记录文件中的数据库名 */
    for (int number = 0 ; number < tablesRecordPathList.size() ; number++)
        FileUtil().replaceContent(tablesRecordPathList[number] , oldName , newName);

    /* 更改所有表结构文件中的数据库名 */
    for (int number = 0 ; number < tablesStructurePathList.size() ; number++)
        FileUtil().replaceContent(tablesStructurePathList[number] , oldName , newName);

    /* 更改数据库描述文件中的数据库名 */
    FileUtil().replaceContent(dbPath , oldName , newName);

    /* 更改数据库文件夹名 */
    FileUtil().modifyFolderName(dbFolderPath , dbFolderPathNew);

    /* 更改系统数据库文件中的数据库名 */
    FileUtil().replaceContent(sysdbPath , oldName , newName);

    //更新数据库对应的xxx.db文件的文件名
    //1首先读取oldName.db内容
    dbPath = rootPath;
    dbPath.append(newName + "//" + oldName + ".db");
    QString content = FileUtil().readAll(dbPath);
    //2删除掉原来的文件
    FileUtil().deleteFile(dbPath);
    //3创建新文件(newName.db)
    FileUtil().createFile(dbNewPath);
    //4将内容写入新文件
    FileUtil().writeAllFileForQString(dbNewPath,content);
    return 1;
}
vector<DBEntity> DBDao::getDatabaseList()
{
    vector<DBEntity> DBL;

    /* 打开system.h文件*/
    QString  S = DB_CREATE_PATH;
    S.append("system.db");
    char* SysPath;
    QByteArray ba = S.toLatin1();
    SysPath = ba.data();
    FILE* f;
    if ((f = fopen(SysPath, "r")) == NULL)
    {
        fprintf(stderr, "Failure in opening file: %s\n", SysPath);
        exit(EXIT_FAILURE);
    }

    /* 从文件中读取数据库信息 */
    char c;
    c = getc(f);
    if(c=='0'){
        /* 关闭文件 */
        if (fclose(f) != 0)
        {
            fprintf(stderr, "Error in closing file: %s\n", SysPath);
            exit(EXIT_FAILURE);
        }
        return DBL;
    }
    while ((c = getc(f)) != '\n');// 读取到第二行开头
    while (feof(f) == 0)// 当文件未读到末尾
    {
        DBEntity DBI;

        /* 写入数据库id */
        int id; //数据库id
        fscanf(f, "%d", &id);
        DBI.setId(id);
        c = getc(f);// 清除#号

        /* 写入数据库名 */
        char DBName[100];
        int index = 0;
        while ((c = getc(f)) != '#')
        {
            DBName[index] = c;
            index++;
        }
        DBName[index] = '\0';// 设置字符串末尾
        DBI.setName(QString(QLatin1String(DBName)));

        /* 写入数据库类型 */
        int i;
        fscanf(f, "%d", &i);
        DBI.setType(i);
        c = getc(f);// 清除#号

        /* 读取表个数 */
        int TableNum;
        fscanf(f, "%d", &TableNum);
        DBI.setTableNum(TableNum);
        c = getc(f);// 清除#号

        /* 读取数据库创建时间 */
        long int CreatTime;
        fscanf(f, "%ld", &CreatTime);
        DBI.setCreateTime((time_t)CreatTime);
        c = getc(f);//清除换行符

        /* 路径 */
        QString DBPath = DB_CREATE_PATH;
        DBPath.append(DBI.getName());
        DBI.setFilePath(DBPath);

        /* 加入到列表中 */
        DBL.push_back(DBI);
    }

    /* 关闭文件 */
    if (fclose(f) != 0)
    {
        fprintf(stderr, "Error in closing file: %s\n", SysPath);
        exit(EXIT_FAILURE);
    }



    return DBL;
}
