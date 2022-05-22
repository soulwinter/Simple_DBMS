#include "RecordService.h"
#include <regex>

RecordService::RecordService() {

}

int RecordService::insertRecord(QString dbName, QString tableName, RecordEntity recordEntity) {
    QString resu = DB_CREATE_PATH;
    resu.append(dbName);
    FileUtil f;
    resu.append("//" + tableName + "//");
    resu.append(tableName+".tdf"); // 获取表描述文件的绝对路径

    QString temf = f.readLine(resu, 1); //读取第一行
    QStringList temt = temf.split("#");
    int cou = temt[0].toInt();   // 获取计数器

    vector<FieldEntity> fe;     // FieldEntity容器，储存字段约束
    temt.clear(); //
    temf = "";
    FieldEntity fd;
    for (int i = 2; i <= cou + 1; i++) {
        temf = f.readLine(resu, i);
        temt = temf.split("#");
        int id = temt[0].toInt(); fd.setId(id);
        fd.setName(temt[1]);
        fd.setType(temt[2].toInt());
        fd.setLength(temt[3].toInt());
        fd.setMax(temt[4].toDouble());
        fd.setMin(temt[5].toDouble());
        fd.setDefaultValue(temt[6]);
        if (temt[7] == "1") fd.setIsPK(1);
        else fd.setIsPK(false);
        if (temt[8] == "1") fd.setIsNotNUll(1);
        else fd.setIsNotNUll(0);
        if (temt[9] == "1") fd.setIsUnique(1);
        else fd.setIsUnique(false);
        fd.setComment(temt[10]);
        temt.clear(); //
        temf = "";
        fe.push_back(fd);
    }
    // 初始化字段约束完成
    // 开始判断数据合法性
    map<QString, QString> m = recordEntity.getValueMap();
    for (int i = 0; i < cou; i++) {//字段数目
        QString value = fe[i].getName();  // 获取字段名字,fe[i]对应的就是此字段的所有约束
        map<QString, QString>::iterator iter = m.find(value);
        if (iter != m.end()) {
            temf = iter->second;   // 获取到了字段对应的值
            //开始判断
            if ((temf.length() == 0 || temf.compare("null")==0) && fe[i].getIsNotNUll() == 1) return 8009; //值为空但约束不准为空
            if (temf.length() != 0) {  // 不为空值
                //只有varchar需要判断长度
                if(fe[i].getType()==VARCHAR){
                    if (temf.length() > fe[i].getLength()) return 8008; //长度溢出
                }
                // 判断类型
                if (fe[i].getType() == 1) {  //整型
                    regex intRegex("[0-9]+");
                    if(!regex_match(temf.toStdString(),intRegex)){
                        return 10086;//字段类型不匹配
                    }
                    int k = temf.toInt();
                    //查看是否设置最大最小值(等于以下这个奇怪的数说明没有设置)
                    if(fe[i].getMin()!=-4.83669782722e-26){
                        if (k<fe[i].getMin() || k>fe[i].getMax()) return 8007; // 超过了约束范围(int)
                    }
                }
                if (fe[i].getType() == 2) {
                    if (temf != "true" && temf != "false") return 8007;// 超过了约束范围(bool)
                }
                if (fe[i].getType() == 5) {
                    regex doubleRegex("[0-9]+\.[0-9]+");
                    if(!regex_match(temf.toStdString(),doubleRegex)){
                        return 10086;//字段类型不匹配
                    }
                    double k = temf.toDouble();
                    //查看是否设置最大最小值
                    if(fe[i].getMin()!=-4.83669782722e-26){
                        if (k<fe[i].getMin() || k>fe[i].getMax()) return 8007; // 超过了约束范围(int)
                    }
                }
                if (fe[i].getType() == 3) {
                    regex timeRegex("[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]");
                    if(!regex_match(temf.toStdString(),timeRegex)){
                        return 10086;//字段类型不匹配
                    }
                }
                // 判断主键和唯一
                //从trd文件中获取属性比较主键
                if (fe[i].getIsPK() || fe[i].getIsUnique()) {
                    QString trd = DB_CREATE_PATH;
                    trd.append(dbName);
                    trd.append("//" + tableName + "//"); trd.append(tableName + ".trd");
                    temt.clear();
                    QString trdw = f.readLine(trd, 1);
                    temt = trdw.split("#");
                    int jn = temt[0].toInt();    // 获取计数器
                    temt.clear();
                    trdw = "";
                    for (int o = 2; o <= jn + 1; o++) {  //从trd文件的第二行开始
                        trdw = f.readLine(trd, o);
                        temt = trdw.split("#");
                        if (temt[i+1] == temf && temf.compare("null")!=0) {
                            return 8005; // 主键的数据重复
                        }
                    }
                }
            }
        }
        else return 8010;  // 没读到说明没有这个字段

    }
    // 循环结束，所有字段判断完成
    QString path = DB_CREATE_PATH;
    path.append(dbName);
    return RecordDao().AddRecord(path, tableName, recordEntity);
}

int RecordService::deletrecord(RecordEntity recordEntity, QString dbName,QString tableName) {
    QString path = DB_CREATE_PATH;
    path.append(dbName + "//" + tableName);
    return RecordDao().deleteRecord(path, tableName, recordEntity);
}

int RecordService::modifyrecord(QString dbName, QString tableName, RecordEntity old, RecordEntity ne) {
    QString resu = DB_CREATE_PATH;
    resu.append(dbName + "//" + tableName + "//");
    FileUtil f;
    resu.append(tableName + ".tdf"); // 获取表描述文件的绝对路径

    QString temf = f.readLine(resu, 1); //读取第一行
    QStringList temt = temf.split("#");
    int cou = temt[0].toInt();   // 获取计数器

    vector<FieldEntity> fe;     // FieldEntity容器，储存字段约束
    temt.clear(); //
    temf = "";
    FieldEntity fd;
    for (int i = 2; i <= cou + 1; i++) {
        temf = f.readLine(resu, i);
        temt = temf.split("#");
        int id = temt[0].toInt(); fd.setId(id);
        fd.setName(temt[1]);
        fd.setType(temt[2].toInt());
        fd.setLength(temt[3].toInt());
        fd.setMax(temt[4].toDouble());
        fd.setMin(temt[5].toDouble());
        fd.setDefaultValue(temt[6]);
        if (temt[7] == "1") fd.setIsPK(1);
        else fd.setIsPK(0);
        if (temt[8] == "1") fd.setIsNotNUll(1);
        else fd.setIsNotNUll(0);
        if (temt[9] == "1") fd.setIsUnique(1);
        else fd.setIsUnique(0);
        fd.setComment(temt[10]);
        temt.clear(); //
        temf = "";
        fe.push_back(fd);
    }
    // 初始化字段约束完成
    // 开始判断数据合法性
    map<QString, QString> m = ne.getValueMap();
    for (int i = 0; i < cou; i++) {//字段数目
        QString value = fe[i].getName();  // 获取字段名字,fe[i]对应的就是此字段的所有约束
        map<QString, QString>::iterator iter = m.find(value);
        if (iter != m.end()) {
            temf = iter->second;   // 获取到了字段对应的值
            //开始判断
            if (temf.length() == 0 && fe[i].getIsNotNUll() == 1) return 8009; //值为空但约束不准为空
            if (temf.length() != 0) {  // 不为空值
                //只有varchar需要判断长度
                if(fe[i].getType()==VARCHAR){
                    if (temf.length() > fe[i].getLength()) return 8008; //长度溢出
                }
                // 判断类型
                if (fe[i].getType() == 1) {  //整型
                    regex intRegex("[0-9]+");
                    if(!regex_match(temf.toStdString(),intRegex)){
                        return 10086;//字段类型不匹配
                    }
                    int k = temf.toInt();
                    if (k<fe[i].getMin() || k>fe[i].getMax()) return 8007; // 超过了约束范围(int)
                }
                if (fe[i].getType() == 2) {
                    if (temf != "true" && temf != "false") return 8007;// 超过了约束范围(bool)
                }
                if (fe[i].getType() == 5) {
                    regex doubleRegex("[0-9]+\.[0-9]+");
                    if(!regex_match(temf.toStdString(),doubleRegex)){
                        return 10086;//字段类型不匹配
                    }
                    double k = temf.toDouble();
                    if (k<fe[i].getMin() || k>fe[i].getMax()) return 8007; // 超过了约束范围(double)
                }
                if (fe[i].getType() == 3) {
                    regex timeRegex("[0-9][0-9][0-9][0-9]-[0-9][0-9]-[0-9][0-9]");
                    if(!regex_match(temf.toStdString(),timeRegex)){
                        return 10086;//字段类型不匹配
                    }
                // 判断主键和唯一
                //从trd文件中获取属性比较主键
                if (fe[i].getIsPK() || fe[i].getIsUnique()) {
                    QString trd = DB_CREATE_PATH;
                    trd.append(dbName);
                    trd.append("//" + tableName + "//"); trd.append(tableName + ".trd");
                    temt.clear();
                    QString trdw = f.readLine(trd, 1);
                    temt = trdw.split("#");
                    int jn = temt[0].toInt();    // 获取计数器
                    temt.clear();
                    trdw = "";
                    for (int o = 2; o <= jn + 1; o++) {  //从trd文件的第二行开始
                        trdw = f.readLine(trd, o);
                        temt = trdw.split("#");
                        if (temt[i+1] == temf && temf.compare("null")!=0) {
                            return 8005; // 主键的数据重复
                        }
                    }
                }
            }
        }
        else return 8010;  // 没读到说明没有这个字段

    }
    // 循环结束，所有字段判断完成
    return RecordDao().UpdateRecord(dbName, tableName, old, ne);
}
}

vector<RecordEntity> RecordService::getRecordList(QString dbName, QString tableName){
    return RecordDao().getRecordList(dbName,tableName);
}
