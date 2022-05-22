#include "sqlanalyze.h"

sqlAnalyze::sqlAnalyze(QString databaseName)
{
    this->databaseName = databaseName;
}


int sqlAnalyze::checkInput(QString input){  //这一步时 input是用户输入的一段字符串，可能包含空格，换行符
    input = input.simplified(); //去除换行符和前后空格
    //create insert update delete select
    QChar start = input[0];
    QString path = DB_CREATE_PATH;
    path.append(databaseName);

    switch (start.toLatin1()) {
    case 'c':
        if(input.contains("table")){
            return createTableSql(input,databaseName);
        }
        if(input.contains("database")){
            return createDatebaseSql(input);
        }
        return 0;
        break;
    case 'i':

        return analysql(path,input);
        break;
    case 'u':
        if(checkUpdate(input)){
            //检查通过拆分字符串
            //update sc set x = 0 where x=1 and x=2;
            QString tableName;
            QStringList localSplit = input.split(" ");
            //设置表名
            for(int i=1;i<localSplit.size();i++){
                if(localSplit[i].compare(" ")!=0){
                    tableName = localSplit[i];
                    break;
                }
            }
            //判断一下有没有表
            QString path_tb = DB_CREATE_PATH;
            path_tb.append(databaseName).append("//").append(tableName);
            if(!FileUtil().ifExistFolder(path_tb)){
                return 9001;
            }
            //set部分
            vector<vector<QString>> setField;
            int locationSet = input.indexOf("set");
            int locationWhere = input.indexOf("where");
            QString setContent = input.mid(locationSet+3,locationWhere-locationSet-3);
            if(setContent.at(setContent.size()-1)==';'){
                setContent = setContent.left(setContent.size()-1); //这一步是去除‘;’  结果x = 1 and c=b
            }
            QStringList setContentAfterDeal = setContent.split(',');
            for(int i=0;i<setContentAfterDeal.size();i++){
                //对每一个字段进行处理
                QString fieldAndValue = setContentAfterDeal[i];
                fieldAndValue.remove(QRegularExpression("\\s")); //去除空格  field=value
                QStringList final = fieldAndValue.split('='); //field value
                vector<QString> result;
                result.push_back(final[0]);
                result.push_back(final[1]);
                setField.push_back(result);
            }
            //where部分 where x = 1 and c=b;
            vector<vector<QString>> andField;
            vector<vector<QString>> orField;
            if(locationWhere !=-1){
                QString whereContent = input.mid(locationWhere+5);
                whereContent = whereContent.left(whereContent.size()-1); //这一步是去除‘;’  结果x = 1 and c=b
                //判断一下是or还是and
                if(whereContent.contains("and")){
                    QStringList temp = whereContent.split("and");  //x=1  c=b
                    for(int i=0;i<temp.size();i++){
                        //对每一个字段进行处理
                        QString tempField = temp.at(i);
                        tempField.remove(QRegularExpression("\\s")); //去除空格  x=1
                        QStringList final = tempField.split('=');
                        vector<QString> result;
                        result.push_back(final[0]);
                        result.push_back(final[1]);
                        andField.push_back(result);
                    }
                }else{
                    QStringList temp = whereContent.split("or");  //x=1  c=b
                    for(int i=0;i<temp.size();i++){
                        //对每一个字段进行处理
                        QString tempField = temp.at(i);
                        tempField.remove(QRegularExpression("\\s")); //去除空格  x=1
                        QStringList final = tempField.split('=');
                        vector<QString> result;
                        result.push_back(final[0]);
                        result.push_back(final[1]);
                        orField.push_back(result);
                    }
                }
            }

            //首先根据数据库和表名查到所有记录，和字段
            vector<RecordEntity> recordList = RecordService().getRecordList(databaseName,tableName);
            vector<FieldEntity> fieldList = FieldService().getFieldList(databaseName,tableName);
            //遍历更新
            for(int i=0;i<recordList.size();i++){
                RecordEntity record = recordList.at(i);

                //判断while的条件
                int flag = 1;//看看满不满足where条件
                if(orField.empty()){
                    for(int j = 0;j<andField.size();j++){
                        if(record.getValue(andField.at(j).at(0)).compare(andField.at(j).at(1))!=0){
                            flag = 0;
                            break;
                        }
                    }
                    if(flag){
                        //满足条件进行修改
                        RecordEntity newRecord = record;
                        for(int k =0;k<setField.size();k++){
                            QString field = setField.at(k).at(0);
                            QString value = setField.at(k).at(1);
                            FieldEntity fieldEntity;
                            for(int m =0;m<fieldList.size();m++){
                                if(fieldList.at(m).getName().compare(field)==0){
                                    fieldEntity = fieldList.at(m);
                                    break;
                                }
                            }
                            //首先判断一下字段存不存在
                            map<QString, QString> valueMap = newRecord.getValueMap();
                            if(valueMap.find(field)==valueMap.end()){
                                return 8010;
                            }
                            //看看是不是在原来的基础上操作
                            if(value.indexOf(field)!=-1){
                                //仅支持int和double类型
                                if(fieldEntity.getType()==1){
                                    QString valueOld = record.getValue(field);
                                    QString operatorValue = value;
                                    value = valueOld;
                                    int result = operatorInUpdate(&value,operatorValue,1);
                                    if(result==-1){
                                        return result;
                                    }
                                }else if(fieldEntity.getType()==5){
                                    QString valueOld = record.getValue(field);
                                    QString operatorValue = value;
                                    value = valueOld;
                                    int result = operatorInUpdate(&value,operatorValue,5);
                                    if(result==-1){
                                        return result;
                                    }
                                }else{
                                    return 9002; //其他类型不支持加减乘除运算
                                }
                            }
                            newRecord.SetValue(field,value);
                        }
                        int result = RecordService().modifyrecord(databaseName,tableName,record,newRecord);
                        if(result!=1){
                            return result;
                        }
                    }
                }else{
                    flag = 0;
                    for(int j = 0;j<orField.size();j++){
                        if(record.getValue(orField.at(j).at(0)).compare(orField.at(j).at(1))==0){
                            flag =1;
                            break;
                        }
                    }
                    if(flag){
                        //满足条件进行修改
                        RecordEntity newRecord;
                        newRecord.setId(record.getId());
                        map<QString, QString> mp = record.getValueMap();
                        map<QString, QString>::iterator iter;
                        for(iter = mp.begin();iter != mp.end();iter ++)
                        {
                            newRecord.SetValue(iter->first,iter->second);
                        }
                        for(int k =0;k<setField.size();k++){
                            QString field = setField.at(k).at(0);
                            QString value = setField.at(k).at(1);
                            FieldEntity fieldEntity;
                            for(int m =0;m<fieldList.size();m++){
                                if(fieldList.at(m).getName().compare(field)==0){
                                    fieldEntity = fieldList.at(m);
                                    break;
                                }
                            }
                            //首先判断一下字段存不存在
                            map<QString, QString> valueMap = newRecord.getValueMap();
                            if(valueMap.find(field)==valueMap.end()){
                                return 8010;
                            }
                            //看看是不是在原来的基础上操作
                            if(value.indexOf(field)!=-1){
                                //仅支持int和double类型
                                if(fieldEntity.getType()==1){
                                    QString valueOld = record.getValue(field);
                                    QString operatorValue = value;
                                    value = valueOld;
                                    int result = operatorInUpdate(&value,operatorValue,1);
                                    if(result==-1){
                                        return result;
                                    }
                                }else if(fieldEntity.getType()==5){
                                    QString valueOld = record.getValue(field);
                                    QString operatorValue = value;
                                    value = valueOld;
                                    int result = operatorInUpdate(&value,operatorValue,5);
                                    if(result==-1){
                                        return result;
                                    }
                                }else{
                                    return 9002; //其他类型不支持加减乘除运算
                                }
                            }
                            newRecord.SetValue(field,value);
                        }
                        int result = RecordService().modifyrecord(databaseName,tableName,record,newRecord);
                        if(result!=1){
                         return result;
                        }
                    }
                }
            }

            return true;
        }
        else{
            return 0;
        }
        break;
    case 'd':
        return operatorInDelete(input);
        break;
    case 's':
        return 0;
        break;
    case 'a':
        return operatorInAlterTable(input);
    default:
        return 0;
        break;
    }
    return false;
}

int sqlAnalyze::operatorInUpdate(QString *value, QString operater,int type){
    if(type==1){
        int number = value->toInt();
        if(operater.indexOf('+')!=-1){
            int index = operater.indexOf('+');
            if(operater.at(0).unicode()-'0'<0||operater.at(0).unicode()-'0'>9){
                //说明是 value-数字
                number = number + operater.mid(index+1).toInt();
            }else{
                //说明是 数字-value 5-x
                number = operater.mid(0,index).toInt() + number;
            }
        }else if(operater.indexOf('-')!=-1){
            int index = operater.indexOf('-');
            if(operater.at(0).unicode()-'0'<0||operater.at(0).unicode()-'0'>9){
                //说明是 value-数字
                number = number - operater.mid(index+1).toInt();
            }else{
                //说明是 数字-value 5-x
                number = operater.mid(0,index).toInt() - number;
            }
        }else if(operater.indexOf('*')!=-1){
            int index = operater.indexOf('*');
            if(operater.at(0).unicode()-'0'<0||operater.at(0).unicode()-'0'>9){
                //说明是 value-数字
                number = number * operater.mid(index+1).toInt();
            }else{
                //说明是 数字-value 5-x
                number = operater.mid(0,index).toInt() * number;
            }
        }else if(operater.indexOf('/')!=-1){
            int index = operater.indexOf('/');
            if(operater.at(0).unicode()-'0'<0||operater.at(0).unicode()-'0'>9){
                //说明是 value-数字
                if(operater.mid(index+1).toInt()==0){
                    return -1;
                }
                number = number / operater.mid(index+1).toInt();
            }else{
                //说明是 数字-value 5-x
                if(number==0){
                    return -1;
                }
                number = operater.mid(0,index).toInt() / number;
            }
        }
        *value = QString::number(number);
        return 1;
    }else{
        double number = value->toInt();
        if(operater.indexOf('+')!=-1){
            int index = operater.indexOf('+');
            if(operater.at(0).unicode()-'0'<0||operater.at(0).unicode()-'0'>9){
                //说明是 value-数字
                number = number + operater.mid(index+1).toDouble();
            }else{
                //说明是 数字-value 5-x
                number = operater.mid(0,index).toDouble() + number;
            }
        }else if(operater.indexOf('-')!=-1){
            int index = operater.indexOf('-');
            if(operater.at(0).unicode()-'0'<0||operater.at(0).unicode()-'0'>9){
                //说明是 value-数字
                number = number - operater.mid(index+1).toDouble();
            }else{
                //说明是 数字-value 5-x
                number = operater.mid(0,index).toDouble() - number;
            }
        }else if(operater.indexOf('*')!=-1){
            int index = operater.indexOf('*');
            if(operater.at(0).unicode()-'0'<0||operater.at(0).unicode()-'0'>9){
                //说明是 value-数字
                number = number * operater.mid(index+1).toDouble();
            }else{
                //说明是 数字-value 5-x
                number = operater.mid(0,index).toDouble() * number;
            }
        }else if(operater.indexOf('/')!=-1){
            int index = operater.indexOf('/');
            if(operater.at(0).unicode()-'0'<0||operater.at(0).unicode()-'0'>9){
                //说明是 value-数字
                if(operater.mid(index+1).toDouble()==0.0){
                    return -1;
                }
                number = number / operater.mid(index+1).toDouble();
            }else{
                //说明是 数字-value 5-x
                if(number== 0.0){
                    return -1;
                }
                number = operater.mid(0,index).toDouble() / number;
            }
        }
        *value = QString::number(number);
        return 1;
    }

}

int sqlAnalyze::operatorInDelete(QString input)
{
    string inputString = std::string(input.toLocal8Bit());// 输入字符串的string类型
    bool existWhere = 0;// 判断是否存在where子句
    /* 匹配delete from的变量 */
    string tableName;
    smatch result1;
                    //    delete   from test where
    regex pattern1(" *[Dd][Ee][Ll][Ee][Tt][Ee] +[Ff][Rr][Oo][Mm] +([^ ]+).*");
    /* 匹配where子句的变量 */
    string fieldName;
    string fieldValue;
    smatch result2;
    regex pattern2(".*[Ww][Hh][Ee][Rr][Ee] +([^ ]+) *= *([^; ]+).*");
    FILE* tdfFile;// 表结构文件
    QString trdFilePath = "F:/DBMS/";// 表记录文件路径
    QString tdfFilePath = "F:/DBMS/";// 表结构文件路径
    char* trdFilePathChar;
    char* tdfFilePathChar;
    int fieldNumber = -1;// 记录where子句中的字段是第几个字段，如果没有记为-1

    /* 如果delet from匹配失败，说明语法错误，退出程序 */
    if (regex_search(inputString,result1, pattern1) == 0)
        return 317;
    tableName = result1[1].str();
    tdfFilePath.append(databaseName + "/" + QString::fromStdString(tableName) + "/" + QString::fromStdString(tableName) + ".tdf");
    trdFilePath.append(databaseName + "/" + QString::fromStdString(tableName) + "/" + QString::fromStdString(tableName) + ".trd");
    QByteArray a = tdfFilePath.toLatin1();
    tdfFilePathChar = a.data();
    QByteArray b = trdFilePath.toLatin1();
    trdFilePathChar = b.data();

    /* 判断表是否存在 */
    if (!FileUtil().ifExistFile(trdFilePath))
        return 313;

    /* 匹配where子句 */
    if (regex_match(inputString, result2, pattern2))
    {
        existWhere = 1;
        fieldName = result2[1].str();
        fieldValue = result2[2].str();
    }

    if (existWhere)
    {
        int j = 1;
        vector<RecordEntity> recordList = RecordService().getRecordList(databaseName,QString::fromStdString(tableName));
        vector<RecordEntity> recordListToSave;
        for(int i=0;i<recordList.size();i++){
            RecordEntity record = recordList.at(i);
            if(record.getValue(QString::fromStdString(fieldName)).compare(QString::fromStdString(fieldValue))==0){
               RecordService().deletrecord(record,databaseName,QString::fromStdString(tableName));
            }
        }
    }
    else // 否则删除整个表记录文件除去第一行的所有记录
    {
        QFile trdFile(trdFilePath);
        FileUtil().updateCounter(trdFilePath,0);
        QString content = FileUtil().readLine(trdFilePath,1);
        FileUtil().writeAllFileForQString(trdFilePath,content);
    }

    return 1;
}


bool sqlAnalyze::checkUpdate(QString input){
    regex updateandRegex("update +[^ ]+ +set +[^ ]+ *= *[^ ,]+( *, *[^ ]+ *= *[^ ,])*( +where +[^ ]+ *= *[^ ]+( +and +[^ ]+ *= *[^ ]+)?)? *;");
    regex updateorRegex("update +[^ ]+ +set +[^ ]+ *= *[^ ,]+( *, *[^ ]+ *= *[^ ,])*( +where +[^ ]+ *= *[^ ]+( +or +[^ ]+ *= *[^ ]+)?)? *;");

    return regex_match(input.toStdString(),updateandRegex) || regex_match(input.toStdString(),updateorRegex);
}


int sqlAnalyze::createTableSql(QString content, QString dbName)
{
    /*
      return 1;//正常运行
      return 0;//字段名重复
      return 2;//varchar长度不为数字
      return 3;//字段类型不正确
    */
    string str=content.toStdString();
    regex r("(create)(\\s+)(table)(\\s+)(.*)");
    smatch sm1;
    bool flag = regex_match(str,sm1,r);
    if(flag==1)
    {
        string str2=sm1[5];//表名与后面的字段
        regex r2("([0-9a-zA-Z]*)(\\s*)(\\()(\\s*)([0-9a-zA-Z\\s,()'-.]+)(\\)\\;$)");
        smatch sm2;
        bool flag2=regex_match(str2,sm2,r2);
        vector<FieldEntity> TE;
        TableService TS;
        if(flag2==1)//带有参数能够匹配()的情况下
        {
            QString totalField=QString::fromStdString(sm2[5]);
            QStringList strList=totalField.split(",");
            QString field;QString field1;QString field2;QString field3;
            QString num;
            QString field4;QString field5;QString num2;//primary not null default
            //判断数据类型是否正确
            //判断字段名是否重复
            for(int i=0;i<strList.count();i++)
            {
                FieldEntity FE;
                FE.setIsNotNUll(0);
                regex r3("(\\s*)([0-9a-zA-Z]*)(\\s+)([0-9a-zA-Z]*)");//非varchar
                regex r4("(\\s*)([0-9a-zA-Z]*)(\\s+)([0-9a-zA-Z]*)(\\s*)(\\()(\\s*)([0-9]+)(\\s*)(\\))");

                //primary key not null
                regex r5("(\\s*)([0-9a-zA-Z]*)(\\s+)([0-9a-zA-Z]*)(\\s+)([a-zA-Z]*)(\\s+)([a-zA-Z]*)");//非varchar
                regex r6("(\\s*)([0-9a-zA-Z]*)(\\s+)([0-9a-zA-Z]*)(\\s*)(\\()(\\s*)([0-9]+)(\\s*)(\\))(\\s+)([a-zA-Z]*)(\\s+)([a-zA-Z]*)");
                // default
                regex r7("(\\s*)([0-9a-zA-Z]*)(\\s+)([0-9a-zA-Z]*)(\\s+)([a-zA-Z]*)(\\s+)([0-9.]+)");//非varchar int double
                regex r8("(\\s*)([0-9a-zA-Z]*)(\\s+)([0-9a-zA-Z]*)(\\s+)([a-zA-Z]*)(\\s+)([-'a-z0-9]*)");//非varchar time bool
                regex r9("(\\s*)([0-9a-zA-Z]*)(\\s+)([0-9a-zA-Z]*)(\\s*)(\\()(\\s*)([0-9]+)(\\s*)(\\))(\\s+)([a-zA-Z]*)(\\s+)(['a-z:0-9]*)");
                // unique
                regex r10("(\\s*)([0-9a-zA-Z]*)(\\s+)([0-9a-zA-Z]*)(\\s+)([a-zA-Z]*)");//非varchar
                regex r11("(\\s*)([0-9a-zA-Z]*)(\\s+)([0-9a-zA-Z]*)(\\s*)(\\()(\\s*)([0-9]+)(\\s*)(\\))(\\s+)([a-zA-Z]*)");

                smatch sm3;
                field+=strList.at(i);
                string Field=field.toStdString();
                FE.setType(0);
                if(regex_match(Field,sm3,r3)){
                    field1+=QString::fromStdString(sm3[2]);
                    field2+=QString::fromStdString(sm3[4]);
                    if(field2=="int")
                        FE.setType(1);
                    else if(field2=="bool")
                        FE.setType(2);
                    else if(field2=="time")
                        FE.setType(3);
                    else if(field2=="double")
                        FE.setType(5);
                }
                else if(regex_match(Field,sm3,r4)){

                    field1+=QString::fromStdString(sm3[2]);
                    field2+=QString::fromStdString(sm3[4]);
                    field3+=QString::fromStdString(sm3[8]);
                    if(field2=="varchar")
                        FE.setType(4);
                    for(int j=0;j<field3.length();j++)
                    {
                        if(field3[j]>='0' && field3[j]<='9')
                            num+=field3[j];
                        else{
                            return 2;//varchar长度不为数字
                        }
                    }
                    FE.setLength(num.toInt());
                }
                else if(regex_match(Field,sm3,r5)){
                    field1+=QString::fromStdString(sm3[2]);
                    field2+=QString::fromStdString(sm3[4]);
                    field4+=QString::fromStdString(sm3[6]);
                    field5+=QString::fromStdString(sm3[8]);
                    if(field2=="int")
                        FE.setType(1);
                    else if(field2=="bool")
                        FE.setType(2);
                    else if(field2=="time")
                        FE.setType(3);
                    else if(field2=="double")
                        FE.setType(5);
                    if(field4=="primary" && field5=="key")
                        FE.setIsPK(1);
                    if(field4=="not" && field5=="null")
                        FE.setIsNotNUll(1);
                }
                else if(regex_match(Field,sm3,r6)){
                    field1+=QString::fromStdString(sm3[2]);
                    field2+=QString::fromStdString(sm3[4]);
                    field3+=QString::fromStdString(sm3[8]);
                    field4+=QString::fromStdString(sm3[12]);
                    field5+=QString::fromStdString(sm3[14]);
                    if(field2=="varchar")
                        FE.setType(4);
                    for(int j=0;j<field3.length();j++)
                    {
                        if(field3[j]>='0' && field3[j]<='9')
                            num+=field3[j];
                        else{
                            return 2;//varchar长度不为数字
                        }
                    }
                    FE.setLength(num.toInt());
                    if(field4=="primary" && field5=="key")
                        FE.setIsPK(1);
                    if(field4=="not" && field5=="null")
                        FE.setIsNotNUll(1);
                }
                else if(regex_match(Field,sm3,r7)){
                    field1+=QString::fromStdString(sm3[2]);
                    field2+=QString::fromStdString(sm3[4]);
                    field4+=QString::fromStdString(sm3[6]);
                    field5+=QString::fromStdString(sm3[8]);
                    if(field2=="int")
                        FE.setType(1);
                    else if(field2=="double")
                        FE.setType(5);
                    if(field4=="default"){
                        for(int j=0;j<field5.length();j++)
                        {
                            if(field5[j]=='.' || (field5[j]>='0' && field5[j]<='9'))
                                num2+=field5[j];
                            else{
                                return 4;//default int double长度不为数字
                            }
                        }
                    }
                    FE.setDefaultValue(num2);
                }
                else if(regex_match(Field,sm3,r8)){
                    field1+=QString::fromStdString(sm3[2]);
                    field2+=QString::fromStdString(sm3[4]);
                    field4+=QString::fromStdString(sm3[6]);
                    field5+=QString::fromStdString(sm3[8]);
                    if(field2=="bool")
                        FE.setType(2);
                    else if(field2=="time")
                        FE.setType(3);
                    if(field4=="default"){
                        if(field2=="bool"){
                            if(field5=="'true'")
                                FE.setDefaultValue("true");
                            else if(field5=="'false'")
                                FE.setDefaultValue("false");
                            else
                                return 5;//default bool 不为 'true' 'false'
                        }
                        if(field2=="time"){
                            for(int j=0;j<field5.length();j++)
                            {
                                if(j==5 || j==8 ||j==0 || j==11 ){
                                    if(field5[j]=='-')
                                        num2+=field5[j];
                                }
                                else if((field5[j]>='0' && field5[j]<='9'))
                                    num2+=field5[j];
                                else{
                                    return 6;//default time不满足要求
                                }
                            }
                            if(num2.length()!=10)
                                return 7;//default time不满足要求
                            FE.setDefaultValue(num2);
                        }
                    }
                }
                else if(regex_match(Field,sm3,r9)){
                    field1+=QString::fromStdString(sm3[2]);
                    field2+=QString::fromStdString(sm3[4]);
                    field3+=QString::fromStdString(sm3[8]);
                    field4+=QString::fromStdString(sm3[12]);
                    field5+=QString::fromStdString(sm3[14]);
                    if(field2=="varchar")
                        FE.setType(4);
                    for(int j=0;j<field3.length();j++)
                    {
                        if(field3[j]>='0' && field3[j]<='9')
                            num+=field3[j];
                        else{
                            return 2;//varchar长度不为数字
                        }
                    }
                    FE.setLength(num.toInt());
                    if(field4=="default"){
                        for(int k=1;k<field5.length()-1;k++){
                            num2+=field5[k];
                        }
                        FE.setDefaultValue(num2);
                    }
                }
                else if(regex_match(Field,sm3,r10)){
                    field1+=QString::fromStdString(sm3[2]);
                    field2+=QString::fromStdString(sm3[4]);
                    field4+=QString::fromStdString(sm3[6]);
                    if(field2=="int")
                        FE.setType(1);
                    else if(field2=="bool")
                        FE.setType(2);
                    else if(field2=="time")
                        FE.setType(3);
                    else if(field2=="double")
                        FE.setType(5);
                    if(field4=="unique")
                        FE.setIsUnique(1);
                }
                else if(regex_match(Field,sm3,r11)){
                    field1+=QString::fromStdString(sm3[2]);
                    field2+=QString::fromStdString(sm3[4]);
                    field3+=QString::fromStdString(sm3[8]);
                    field4+=QString::fromStdString(sm3[12]);
                    if(field2=="varchar")
                        FE.setType(4);
                    for(int j=0;j<field3.length();j++)
                    {
                        if(field3[j]>='0' && field3[j]<='9')
                            num+=field3[j];
                        else{
                            return 2;//varchar长度不为数字
                        }
                    }
                    FE.setLength(num.toInt());
                    if(field4=="unique")
                        FE.setIsUnique(1);
                }
                for(int k=0;k<TE.size();k++)
                {
                    if(field1==TE[k].getName())
                        return 0;//字段名重复
                }
                if(FE.getType()==0)
                    return 3;//字段类型不正确
                FE.setName(field1);
                FE.setId(i+1);
                TE.push_back(FE);
                field.clear();field1.clear();field2.clear();field3.clear();num.clear();FE.setLength(0);
                field4.clear();field5.clear();num2.clear();
            }
            TS.setDatabaseName(dbName);
            TS.CreateTable(QString::fromStdString(sm2[1]),TE);
        }
        else
        {
            TS.setDatabaseName(dbName);
            TS.CreateTable(QString::fromStdString(sm1[5]),TE);
        }
        return 1;//正常运行
    }
}

int sqlAnalyze::createDatebaseSql(QString content)
{
    string str=content.toStdString();
    regex r(" *(create)(\\s)*(database)(\\s)*(.+)");
    smatch sm1;
    bool flag = regex_match(str,sm1,r);
    if(flag==1)
    {

        QString dbName=QString::fromStdString(sm1[5]);
        int result =  DBService().createDatabase(dbName);
        if(result){
            return result;
        }else{
            return 500;
        }
    }
    return 0;
}

int sqlAnalyze::analysql(QString dbpath,QString s) {  //在哪个数据库中查询
    QStringList res = s.split(" "); // 将语句按空格分成数组
    int len = 0;   // 语句中单词的个数
    vector<QString> sql;  // 除去输入的语句中的空格
    for (int i = 0; i < res.length(); i++) {
        if (res[i] != "") {
            sql.push_back(res[i]);
            len++;
        }
    }
    if (sql[0].compare("insert", Qt::CaseInsensitive) == 0) {  // 不区分大小写比较
        return insertSql(dbpath, sql);
    }
    return 0;
}


int sqlAnalyze::insertSql(QString dbpath, vector<QString> sql)
{
    if (sql[0].compare("insert", Qt::CaseInsensitive) != 0 || sql[1].compare("into", Qt::CaseInsensitive) != 0 || sql[4].compare("values", Qt::CaseInsensitive) != 0)
        return 102; // 输入违规
    FileUtil f;
    // insert into xxx (s,s,s,s) values (ss,s,s,s) length = 6
    QString resu = dbpath;
    resu.append("//" + sql[2] + "//"+ sql[2]+".tdf");
    if (!f.ifExistFile(resu)) return 103; // 文件夹不存在
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
        if (temt[7] == "true") fd.setIsPK(true);
        else fd.setIsPK(false);
        if (temt[8] == "true") fd.setIsNotNUll(true);
        else fd.setIsNotNUll(false);
        if (temt[9] == "true") fd.setIsUnique(true);
        else fd.setIsUnique(false);
        fd.setComment(temt[10]);
        temt.clear(); //
        temf = "";
        fe.push_back(fd);
    }
    temf = sql[3];
    temf = temf.left(temf.size() - 1);
    temf = temf.right(temf.size() - 1);
    QStringList dest = temf.split(","); // 获取到了括号内指定的字段

    temf = "";
    temf = sql[5];
    temf = temf.left(temf.size() - 1);
    temf = temf.right(temf.size() - 1);
    QStringList cor = temf.split(",");   // 获取字段对应的值

    if (dest.size() != cor.size()) return 102; // 输入违规

    for (int i = 0; i < dest.length(); i++) {  // 循环括号内出现的字段
        temf.clear(); temt.clear();
        int index = 0;   // 找到此字段在fe容器中的位置
        for (; index < fe.size() && dest[i] != fe[index].getName(); index++);
        if (index == fe.size()) return 101; // 表中没有这个字段
        else {   // 有这个字段
            temf = cor[i];   //字段对应的值
            if ((temf.length() == 0 || temf.compare("null") == 0) && fe[i].getIsNotNUll() == 0) return 8009; //值为空但约束不准为空
            if (temf.length() != 0 && temf.compare("null")!=0) {  // 不为空值
                if (fe[index].getType()==VARCHAR && temf.length() > fe[index].getLength()) return 8008; //长度溢出
                // 判断类型
                if (fe[index].getType() == 1) {  //整型
                    int k = temf.toInt();
                    if (k<fe[i].getMin() || k>fe[i].getMax()) return 8007; // 超过了约束范围(int)
                }
                if (fe[index].getType() == 2) {
                    if (temf != "true" && temf != "false") return 8006;// 超过了约束范围(bool)
                }
                if (fe[index].getType() == 3) {
                    double k = temf.toDouble();
                    if (k<fe[index].getMin() || k>fe[index].getMax()) return 8005; // 超过了约束范围(double)
                }
                if (fe[index].getType() == 5) {
                    long k = temf.toLong();
                    if (k<fe[index].getMin() || k>fe[index].getMax()) return 8004; //超过了约束范围(time)
                }
                // 判断主键和唯一
                //从trd文件中获取属性比较主键
                if (fe[index].getIsPK() || fe[index].getIsUnique()) {
                    QString trd = dbpath;
                    trd.append("//" + sql[2] + "//"); trd.append(sql[2] + ".trd");
                    temt.clear();
                    QString trdw = f.readLine(trd, 1);
                    temt = trdw.split("#");
                    int jn = temt[0].toInt();    // 获取计数器
                    temt.clear();
                    for (int o = 2; o <= jn + 1; o++) {  //从trd文件的第二行开始
                        trdw = f.readLine(trd, o);
                        temt = trdw.split("#");
                        if (temt[i + 1] == temf) {
                            return 8003; // 主键的数据重复
                        }
                    }
                }
            }
            fe.erase(fe.begin()+index);
        }
    }
    // 指定的字段判断完成
    //开始判断未指定的字段
    for (int i = 0; i < fe.size(); i++) {
        temf.clear();
        temf = fe[i].getDefaultValue();   // 获取字段默认值
        if (fe[i].getIsNotNUll() == 0 && (temf.length() == 0 || temf.compare("null") == 0)) {   //不准为空
            return 8009;
        }
        dest.push_back(fe[i].getName());
        cor.push_back(temf);
    }
    RecordEntity r;
    for (int i = 0; i < dest.length(); i++) {
        r.SetValue(dest[i], cor[i]);
    }
    QString path = dbpath;
    path.append("//"+sql[2]);
    return RecordService().insertRecord(databaseName,sql[2],r);
}

vector<RecordEntity> sqlAnalyze::selectSql(QString dbName, QString input){
    
    input = input.simplified(); //去除换行符和前后空格
    regex selectAndRegex(" *select +[a-zA-Z0-9\*]+ *(, *[a-zA-Z0-9\*]+)* +from +[a-zA-Z0-9]+( +where +[^ ]+ *= *[^ ]+( +and +[^ ]+ *= *[^ ]+)*)? *");
    regex selectOrRegex(" *select +[a-zA-Z0-9\*]+ *(, *[a-zA-Z0-9\*]+)* +from +[a-zA-Z0-9]+( +where +[^ ]+ *= *[^ ]+( +or +[^ ]+ *= *[^ ]+)*)? *");
    
    vector<RecordEntity> result;
    if(regex_match(input.toStdString(),selectAndRegex)){
        //检查通过拆分字符串
        //select aaa  ,bb ,  c from test where x =1 and y =2 and c =5;
        vector<QString> needView;
        QString tableName;
        if(input.contains("where")){
            vector<vector<QString>> conditions;
            //开始解析
            //1.解析需要展示的字段
            int selectL = input.indexOf("select");
            int formL = input.indexOf("from");
            QString viewContent = input.mid(selectL+6,formL-selectL-6);
            viewContent = viewContent.simplified();
            viewContent = viewContent.remove(QRegularExpression("\\s")); //去除空格 aaa,bb,cc
            QStringList views= viewContent.split(',');
            for(int i =0;i<views.size();i++){
                needView.push_back(views[i]);
            }
            //2.解析表名
            int whereL = input.indexOf("where");
            QString tableContent = input.mid(formL+4,whereL-formL-4);
            tableName = tableContent.simplified();
            //3.判断表名是不是存在
            QString path = DB_CREATE_PATH;
            path.append(dbName + "//" + tableName);
            if(!FileUtil().ifExistFolder(path)){
                RecordEntity status;
                status.SetValue("status","402"); //不存在表
                result.push_back(status);
                return result;
            }
            //4.判断用户要展示的字段是不是有不存在的
            vector<FieldEntity> fieldList = FieldService().getFieldList(databaseName,tableName);
            for(int i=0;i<needView.size();i++){
                QString fieldView = needView.at(i);
                int flag = 0;
                for(int j=0;j<fieldList.size();j++){
                    FieldEntity field = fieldList.at(j);
                    if(field.getName().compare(fieldView)==0 || fieldView.compare("*")==0){
                        flag = 1;
                        break;
                    }
                }
                if(!flag){
                    RecordEntity status;
                    status.SetValue("status","401"); //不存在字段
                    result.push_back(status);
                    return result;
                }
            }

            //5解析and部分
            QString andContent = input.mid(whereL+5);
            andContent = andContent.simplified();
            andContent = andContent.remove(QRegularExpression("\\s")); //去除空格 x=1andy=2andc=6
            QStringList andList = andContent.split("and");//[x=1,y=2,c=6]
            for(int i=0;i<andList.size();i++){
                QStringList tempList = andList[i].split('=');
                //对字段名进行检查
                int flag = 0;
                for(int j=0;j<fieldList.size();j++){
                    FieldEntity field = fieldList.at(j);
                    if(field.getName().compare(tempList[0])==0){
                        flag = 1;
                        break;
                    }
                }
                if(!flag){
                    RecordEntity status;
                    status.SetValue("status","401"); //不存在字段
                    result.push_back(status);
                    return result;
                }
                vector<QString> temp;
                temp.push_back(tempList[0]);
                temp.push_back(tempList[1]);
                conditions.push_back(temp);
            }
            //所有解析完成调用方法
            vector<RecordEntity> recordList = RecordService().getRecordList(dbName,tableName);
            for(int i=0;i<recordList.size();i++){
                RecordEntity record = recordList.at(i);
                int flag = 0;
                for(int j=0;j<conditions.size();j++){
                    if(record.getValue(conditions.at(j).at(0)).compare(conditions.at(j).at(1))!=0){
                        flag =1;
                        break;
                    }
                }
                if(!flag){
                    result.push_back(record);
                }
            }
            //填充要显示的字段信息
            RecordEntity recordView;
            for(int i=0;i<needView.size();i++){
                recordView.SetValue(needView.at(i),"ok");
            }
            result.push_back(recordView);
            //状态信息
            RecordEntity status;
            status.SetValue("status","200"); //正确执行
            status.SetValue("tableName",tableName);
            result.push_back(status);
            return result;
        }
        else{
            vector<vector<QString>> conditions;
            //开始解析
            //1.解析需要展示的字段
            int selectL = input.indexOf("select");
            int formL = input.indexOf("from");
            QString viewContent = input.mid(selectL+6,formL-selectL-6);
            viewContent = viewContent.simplified();
            viewContent = viewContent.remove(QRegularExpression("\\s")); //去除空格 aaa,bb,cc
            QStringList views= viewContent.split(',');
            for(int i =0;i<views.size();i++){
                needView.push_back(views[i]);
            }
            //2.解析表名
            QString tableContent = input.mid(formL+4);
            tableName = tableContent.simplified();
            //3.判断表名是不是存在
            QString path = DB_CREATE_PATH;
            path.append(dbName + "//" + tableName);
            if(!FileUtil().ifExistFolder(path)){
                RecordEntity status;
                status.SetValue("status","402"); //不存在表
                result.push_back(status);
                return result;
            }
            //4.判断用户要展示的字段是不是有不存在的
            vector<FieldEntity> fieldList = FieldService().getFieldList(databaseName,tableName);
            for(int i=0;i<needView.size();i++){
                QString fieldView = needView.at(i);
                int flag = 0;
                for(int j=0;j<fieldList.size();j++){
                    FieldEntity field = fieldList.at(j);
                    if(field.getName().compare(fieldView)==0 || fieldView.compare("*")==0){
                        flag = 1;
                        break;
                    }
                }
                if(!flag){
                    RecordEntity status;
                    status.SetValue("status","401"); //不存在字段
                    result.push_back(status);
                    return result;
                }
            }
            //所有解析完成调用方法
            vector<RecordEntity> recordList = RecordService().getRecordList(dbName,tableName);
            for(int i=0;i<recordList.size();i++){
                RecordEntity record = recordList.at(i);
                int flag = 0;
                for(int j=0;j<conditions.size();j++){
                    if(record.getValue(conditions.at(j).at(0)).compare(conditions.at(j).at(1))!=0){
                        flag =1;
                        break;
                    }
                }
                if(!flag){
                    result.push_back(record);
                }
            }
            //填充要显示的字段信息
            RecordEntity recordView;
            for(int i=0;i<needView.size();i++){
                recordView.SetValue(needView.at(i),"ok");
            }
            result.push_back(recordView);
            //状态信息
            RecordEntity status;
            status.SetValue("status","200"); //正确执行
            status.SetValue("tableName",tableName);
            result.push_back(status);
            return result;
        }
    }else if(regex_match(input.toStdString(),selectOrRegex)){
        //检查通过拆分字符串
        //select aaa  ,bb ,  c from test where x =1 and y =2 and c =5;
        vector<QString> needView;
        QString tableName;
        if(input.contains("where")){
            vector<vector<QString>> conditions;
            //开始解析
            //1.解析需要展示的字段
            int selectL = input.indexOf("select");
            int formL = input.indexOf("from");
            QString viewContent = input.mid(selectL+6,formL-selectL-6);
            viewContent = viewContent.simplified();
            viewContent = viewContent.remove(QRegularExpression("\\s")); //去除空格 aaa,bb,cc
            QStringList views= viewContent.split(',');
            for(int i =0;i<views.size();i++){
                needView.push_back(views[i]);
            }
            //2.解析表名
            int whereL = input.indexOf("where");
            QString tableContent = input.mid(formL+4,whereL-formL-4);
            tableName = tableContent.simplified();
            //3.判断表名是不是存在
            QString path = DB_CREATE_PATH;
            path.append(dbName + "//" + tableName);
            if(!FileUtil().ifExistFolder(path)){
                RecordEntity status;
                status.SetValue("status","402"); //不存在表
                result.push_back(status);
                return result;
            }
            //4.判断用户要展示的字段是不是有不存在的
            vector<FieldEntity> fieldList = FieldService().getFieldList(databaseName,tableName);
            for(int i=0;i<needView.size();i++){
                QString fieldView = needView.at(i);
                int flag = 0;
                for(int j=0;j<fieldList.size();j++){
                    FieldEntity field = fieldList.at(j);
                    if(field.getName().compare(fieldView)==0 || fieldView.compare("*")==0){
                        flag = 1;
                        break;
                    }
                }
                if(!flag){
                    RecordEntity status;
                    status.SetValue("status","401"); //不存在字段
                    result.push_back(status);
                    return result;
                }
            }

            //5解析or部分
            QString andContent = input.mid(whereL+5);
            andContent = andContent.simplified();
            andContent = andContent.remove(QRegularExpression("\\s")); //去除空格 x=1andy=2andc=6
            QStringList andList = andContent.split("or");//[x=1,y=2,c=6]
            for(int i=0;i<andList.size();i++){
                QStringList tempList = andList[i].split('=');
                //对字段名进行检查
                int flag = 0;
                for(int j=0;j<fieldList.size();j++){
                    FieldEntity field = fieldList.at(j);
                    if(field.getName().compare(tempList[0])==0){
                        flag = 1;
                        break;
                    }
                }
                if(!flag){
                    RecordEntity status;
                    status.SetValue("status","401"); //不存在字段
                    result.push_back(status);
                    return result;
                }
                vector<QString> temp;
                temp.push_back(tempList[0]);
                temp.push_back(tempList[1]);
                conditions.push_back(temp);
            }
            //所有解析完成调用方法
            vector<RecordEntity> recordList = RecordService().getRecordList(dbName,tableName);
            for(int i=0;i<recordList.size();i++){
                RecordEntity record = recordList.at(i);
                int flag = 0;
                for(int j=0;j<conditions.size();j++){
                    if(record.getValue(conditions.at(j).at(0)).compare(conditions.at(j).at(1))==0){
                        flag =1;
                        break;
                    }
                }
                if(flag){
                    result.push_back(record);
                }
            }
            //填充要显示的字段信息
            RecordEntity recordView;
            for(int i=0;i<needView.size();i++){
                recordView.SetValue(needView.at(i),"ok");
            }
            result.push_back(recordView);
            //状态信息
            RecordEntity status;
            status.SetValue("status","200"); //正确执行
            status.SetValue("tableName",tableName);
            result.push_back(status);
            return result;
        }
        else{
            vector<vector<QString>> conditions;
            //开始解析
            //1.解析需要展示的字段
            int selectL = input.indexOf("select");
            int formL = input.indexOf("from");
            QString viewContent = input.mid(selectL+6,formL-selectL-6);
            viewContent = viewContent.simplified();
            viewContent = viewContent.remove(QRegularExpression("\\s")); //去除空格 aaa,bb,cc
            QStringList views= viewContent.split(',');
            for(int i =0;i<views.size();i++){
                needView.push_back(views[i]);
            }
            //2.解析表名
            QString tableContent = input.mid(formL+4);
            tableName = tableContent.simplified();
            //3.判断表名是不是存在
            QString path = DB_CREATE_PATH;
            path.append(dbName + "//" + tableName);
            if(!FileUtil().ifExistFolder(path)){
                RecordEntity status;
                status.SetValue("status","402"); //不存在表
                result.push_back(status);
                return result;
            }
            //4.判断用户要展示的字段是不是有不存在的
            vector<FieldEntity> fieldList = FieldService().getFieldList(databaseName,tableName);
            for(int i=0;i<needView.size();i++){
                QString fieldView = needView.at(i);
                int flag = 0;
                for(int j=0;j<fieldList.size();j++){
                    FieldEntity field = fieldList.at(j);
                    if(field.getName().compare(fieldView)==0 || fieldView.compare("*")==0){
                        flag = 1;
                        break;
                    }
                }
                if(!flag){
                    RecordEntity status;
                    status.SetValue("status","401"); //不存在字段
                    result.push_back(status);
                    return result;
                }
            }
            //所有解析完成调用方法
            vector<RecordEntity> recordList = RecordService().getRecordList(dbName,tableName);
            for(int i=0;i<recordList.size();i++){
                RecordEntity record = recordList.at(i);
                int flag = 0;
                for(int j=0;j<conditions.size();j++){
                    if(record.getValue(conditions.at(j).at(0)).compare(conditions.at(j).at(1))!=0){
                        flag =1;
                        break;
                    }
                }
                if(!flag){
                    result.push_back(record);
                }
            }
            //填充要显示的字段信息
            RecordEntity recordView;
            for(int i=0;i<needView.size();i++){
                recordView.SetValue(needView.at(i),"ok");
            }
            result.push_back(recordView);
            //状态信息
            RecordEntity status;
            status.SetValue("status","200"); //正确执行
            status.SetValue("tableName",tableName);
            result.push_back(status);
            return result;
        }

    }else{
        RecordEntity status;
        status.SetValue("status","400"); //语句有错
        result.push_back(status);
        return result;
    }
}

vector<RecordEntity> sqlAnalyze::selectSqlWithJoin(QString dbName, QString input){
    input = input.simplified(); //去除换行符和前后空格
    //以下是无聚组版本
    regex selectAndRegex(" *select +.+\..+ *(, *.+\..+)* +from +[a-zA-Z0-9]+( +join +.+ +on +.+\..+=.+\..+)+( +where +.+\..+ *[<>=(>=)(<=)] *.+\..+( +and +.+\..+ *[<>=(>=)(<=)] *.+\..+)*)?");
    regex selectOrRegex(" *select +.+\..+ *(, *.+\..+)* +from +[a-zA-Z0-9]+( +join +.+ +on +.+\..+=.+\..+)+( +where +.+\..+ *[<>=(>=)(<=)] *.+\..+( +or +.+\..+ *[<>=(>=)(<=)] *.+\..+)*)?");
    regex selectAndRegexWithXing(" *select +[\*] +from +[a-zA-Z0-9]+( +join +.+ +on +.+\..+=.+\..+)+( +where +.+\..+ *[<>=(>=)(<=)] *.+\..+( +and +.+\..+ *[<>=(>=)(<=)] *.+\..+)*)?");
    regex selectOrRegexWithXing(" *select +[\*] +from +[a-zA-Z0-9]+( +join +.+ +on +.+\..+=.+\..+)+( +where +.+\..+ *[<>=(>=)(<=)] *.+\..+( +or +.+\..+ *[<>=(>=)(<=)] *.+\..+)*)?");
    //以下是有聚组版本
    regex selectComplexAndRegex(" *select +.+\..+ *(, *.+\..+)* +from +[a-zA-Z0-9]+( +join +.+ +on +.+\..+=.+\..+)+( +where +.+\..+ *[<>=(>=)(<=)] *.+\..+( +and +.+\..+ *[<>=(>=)(<=)] *.+\..+)*)? +group +by +.+\..+");
    regex selectComplexOrRegex(" *select +.+\..+ *(, *.+\..+)* +from +[a-zA-Z0-9]+( +join +.+ +on +.+\..+=.+\..+)+( +where +.+\..+ *[<>=(>=)(<=)] *.+\..+( +or +.+\..+ *[<>=(>=)(<=)] *.+\..+)*)? +group +by +.+\..+");
    regex selectComplexAndRegexWithXing(" *select +[\*] +from +[a-zA-Z0-9]+( +join +.+ +on +.+\..+=.+\..+)+( +where +.+\..+ *[<>=(>=)(<=)] *.+\..+( +and +.+\..+ *[<>=(>=)(<=)] *.+\..+)*)? +group +by +.+\..+");
    regex selectComplexOrRegexWithXing(" *select +[\*] +from +[a-zA-Z0-9]+( +join +.+ +on +.+\..+=.+\..+)+( +where +.+\..+ *[<>=(>=)(<=)] *.+\..+( +or +.+\..+ *[<>=(>=)(<=)] *.+\..+)*)? +group +by +.+\..+");


    /*
    *思路：
    * 分2种情况，有聚组函数，无聚组函数
    * 一.无聚组函数
    *   1.拆分成四部分:需要展示的字段needView，所有表tableNames，连接条件cCondition，选择条件sCondition
    *   2.现根据表把所有记录做笛卡尔积
    *   3.根据连接条件去删除记录
    *   4.根据选择条件去删除记录
    * 二.有聚组函数
    */
    vector<RecordEntity> result;
    int ifMatch = 0; //0不匹配，1匹配简单，2匹配包含聚组
    if(regex_match(input.toStdString(),selectAndRegex) || regex_match(input.toStdString(),selectOrRegex)|| regex_match(input.toStdString(),selectAndRegexWithXing)|| regex_match(input.toStdString(),selectOrRegexWithXing)){
        ifMatch = 1;
        if(input.contains("by")){
            ifMatch = 2;
        }
    }else if(regex_match(input.toStdString(),selectComplexAndRegex) || regex_match(input.toStdString(),selectComplexOrRegex)||regex_match(input.toStdString(),selectComplexAndRegexWithXing) || regex_match(input.toStdString(),selectComplexOrRegexWithXing)){
        ifMatch = 2;
    }
    if(ifMatch!=0){
        //检查通过拆分字符串
        //select sno,grade from student join sc on student.sno = sc.sno
        //select sno,sum(grade),min(grade)  from sc join mm on mm.cc=sc.cc join cc on cc.ss=mm.xx  where cc.x=5 and xx.ss =6 and mm.b < 7 group by sno;
        vector<QString> needView;
        vector<QString> tableNames;
        vector<vector<RecordEntity>> records;
        map<QString, QString> cCondition;
        vector<vector<QString>> sCondition;
        int ifand = 1;
        int groupIndex = input.indexOf("group");
        /*
        *一：拆分 
        */
        //1.解析needView
        int fromIndex = input.indexOf("from");
        QString content = input.mid(6,fromIndex-6); // sno,grade
        content = content.simplified();
        content = content.remove(QRegularExpression("\\s")); //去除空格 sno,grade
        QStringList views= content.split(',');
        for(int i =0;i<views.size();i++){
            needView.push_back(views[i]);
        }
        //2.解析表
        int whereIndex = input.indexOf("where");
        if(whereIndex == -1) whereIndex = input.length();
        content = input.mid(fromIndex+4,whereIndex-fromIndex-4);
        QStringList tableContents= content.split("join");
        for(int i =0;i<tableContents.size();i++){
            QString tableName = "";
            QString each = tableContents.at(i);
            each = each.simplified(); //去除前后空格
            if(i == 0){
              tableName = each;
            }else{
              tableName = each.split(' ').at(0);
            }
            //判断表名是否存在
            QString path = DB_CREATE_PATH;
            path.append(dbName + "//" + tableName);
            if(!FileUtil().ifExistFolder(path)){
                RecordEntity status;
                status.SetValue("status","402"); //不存在表
                result.push_back(status);
                return result;
            }
            tableNames.push_back(tableName);
        }
        //3.解析连接条件
        //on mm.cc=sc.cc join cc on cc.ss=mm.xx join xx on xx.a=cc.b
        int onIndex = input.indexOf("on");
        if(input.contains("where")){
            content = input.mid(onIndex+2,whereIndex-onIndex-2);
        }else if(input.contains("group")){
            content = input.mid(onIndex+2,groupIndex-onIndex-2);
        }else{
            content = input.mid(onIndex+2);
        }

        QStringList conStringList = content.split("join");
        for(int i =0;i<conStringList.length();i++){
            QString each = conStringList.at(i);
            if(i==0){
               each = each.simplified();
               each = each.remove(QRegularExpression("\\s"));
               cCondition[each.split('=').at(0)] = each.split('=').at(1);
            }else{
               // cc on cc.ss=mm.xx 
               each  = each.split("on").at(each.split("on").size()-1);
               each = each.simplified();
               each = each.remove(QRegularExpression("\\s"));
               cCondition[each.split('=').at(0)] = each.split('=').at(1);
            }
        }
        //4.解析选择条件
        if(input.contains("where")){
            QString andContent = "";
            if(input.contains("group")){
                andContent = input.mid(whereIndex+5,groupIndex-whereIndex-5);
            }else{
                andContent = input.mid(whereIndex+5);
            }
            andContent = andContent.simplified();
            andContent = andContent.remove(QRegularExpression("\\s")); //去除空格 x=1andy=2andc=6
            QStringList andList;
            if(andContent.contains("and")){
                andList = andContent.split("and");//[x=1,y<2,c>6]
            }else{
                ifand = 0;
                andList = andContent.split("or");//[x=1,y<2,c>6]
            }

            for(int i = 0;i< andList.size();i++){
                QString each = andList.at(i);
                QString fuhao = "";
                vector<QString> condition;
                if(each.contains("<")){
                    fuhao = "<";
                }else if(each.contains("=")){
                    fuhao = "=";
                }else{
                    fuhao = ">";
                }
                QStringList localSplit = each.split(fuhao);
                condition.push_back(localSplit.at(0));
                condition.push_back(fuhao);
                condition.push_back(localSplit.at(1));
                sCondition.push_back(condition);
            }
        }
        /*
        *二.现根据表把所有记录做笛卡尔积
        */
        //1.取出所有记录
        for(int i =0;i<tableNames.size();i++){
            QString tableName = tableNames.at(i);
            vector<RecordEntity> recordToSave;
            vector<RecordEntity> recordList = RecordService().getRecordList(dbName,tableName);
            for(int j=0;j<recordList.size();j++){
                RecordEntity record = recordList.at(j);
                RecordEntity newRecord;
                //遍历其map，把字段名全部改成tableName.field的格式
                map<QString, QString> mp = record.getValueMap();
                map<QString, QString>::iterator iter;
                for(iter = mp.begin();iter != mp.end();iter ++)
                {
                    newRecord.SetValue(tableName + "." + iter->first,iter->second);
                }
                recordToSave.push_back(newRecord);
            }
            records.push_back(recordToSave);
        }
        //2.进行笛卡尔积
        int nowdeal1 = 0,nowdeal2 = 1;
        int times = records.size()-1;//有3个结果的话，只需要做3-1次笛卡尔积
        for(int i = 0;i<times;i++){
            vector<RecordEntity> recordTemp;
            vector<RecordEntity> recordofNowdeal1 = records.at(nowdeal1);
            for(int j=0;j<recordofNowdeal1.size();j++){
                RecordEntity record1 = recordofNowdeal1.at(j);
                vector<RecordEntity> recordofNowdeal2 = records.at(nowdeal2);
                for(int m=0;m<recordofNowdeal2.size();m++){
                    RecordEntity record2 = recordofNowdeal2.at(m);
                    RecordEntity recordSave = record1;
                    //遍历其map
                    map<QString, QString> mp = record2.getValueMap();
                    map<QString, QString>::iterator iter;
                    for(iter = mp.begin();iter != mp.end();iter ++)
                    {
                        recordSave.SetValue(iter->first,iter->second);
                    }
                    //将笛卡尔积后的结果添加到临时vetcor
                    recordTemp.push_back(recordSave);
                }
            }
            records.push_back(recordTemp);
            nowdeal1 = records.size()-1;
            nowdeal2++;
        }
        /*
        *三.根据连接条件筛选结果
        */
        vector<RecordEntity> recordTemp = records.at(records.size()-1);
        vector<RecordEntity> recordSave;
        for(int i=0;i<recordTemp.size();i++){
            RecordEntity record = recordTemp.at(i);
            int flag = 1;
            //map<QString, QString> cCondition;
            //遍历cCondition
            map<QString, QString>::iterator iter;
            for(iter = cCondition.begin();iter != cCondition.end();iter ++)
            {
                QString field1 = iter->first;
                QString field2 = iter->second;
                map<QString, QString> valueMap = record.getValueMap();
                if(valueMap.find(field1)==valueMap.end()||valueMap.find(field2)==valueMap.end()){
                    RecordEntity status;
                    status.SetValue("status","401"); //不存在字段
                    result.push_back(status);
                    return result;
                }
                if(record.getValue(field1).compare(record.getValue(field2))!=0){
                    flag = 0;
                    break;
                }
            }
            if(flag){
                recordSave.push_back(record);
            }
        }
        records.push_back(recordSave);
        /*
        *四.根据选择条件筛选结果
        */
        vector<RecordEntity> recordSTemp = records.at(records.size()-1);
        for(int i=0;i<recordSTemp.size();i++){
            RecordEntity record = recordSTemp.at(i);
            int flag = 1;
            if(ifand){
                for(int j = 0;j< sCondition.size();j++){
                    vector<QString> tempVector = sCondition.at(j); //["sc.x","<","6"]
                    map<QString, QString> valueMap = record.getValueMap();
                    if(valueMap.find(tempVector.at(0))==valueMap.end()){
                        RecordEntity status;
                        status.SetValue("status","401"); //不存在字段
                        result.push_back(status);
                        return result;
                    }
                    if(!ifSatisfyCondition(record,tempVector)){
                        flag = 0;
                        break;
                    }
                }
            }else{
                flag = 0;
                for(int j = 0;j< sCondition.size();j++){
                    vector<QString> tempVector = sCondition.at(j); //["sc.x","<","6"]
                    map<QString, QString> valueMap = record.getValueMap();
                    if(valueMap.find(tempVector.at(0))==valueMap.end()){
                        RecordEntity status;
                        status.SetValue("status","401"); //不存在字段
                        result.push_back(status);
                        return result;
                    }
                    if(ifSatisfyCondition(record,tempVector)){
                        flag = 1;
                        break;
                    }
                }
            }
            if(flag){
                result.push_back(record);
            }
        }
        /*
        *五.处理最后结果，进行返回
        */
        if(ifMatch==1){
            return dealResult(needView,result,records.size());
        }else{
            //带有聚组函数
            //1.先把group by后面的内容取出来
            int byIndex = input.indexOf("by");
            content = input.mid(byIndex+2); // sc.sno
            content = content.simplified(); //sc.sno
            //2.检查非聚组函数字段是不是与group by后面的相同，并取出需要聚组的字段
            vector<QString> needJuzu;
            for(int i=0;i<needView.size();i++){
                QString field = needView.at(i);
                if(!field.contains("sum")&&!field.contains("avg")&&!field.contains("min")&&!field.contains("max")&&!field.contains("count")){
                    if(field.compare(content)!=0){
                        RecordEntity status;
                        status.SetValue("status","403"); //普通字段必须出现在聚组函数中
                        result.push_back(status);
                        return result;
                    }
                }else{
                    needJuzu.push_back(field);
                }
            }
            //3.调用处理聚组函数的接口，传入content,需要聚组的字段needJuzu,还有结果集result
            //sum
            map<QString, QString> m;
            for(int i=0;i<needJuzu.size();i++){
                QString each = needJuzu[i];
                if(each.contains("sum")){
                    QStringList localSplit = each.split('(');
                    QString value = localSplit.at(localSplit.size()-1); //grade)
                    value = value.left(value.size()-1);
                    m["sum"] = value;
                }
            }
            //avg
            for(int i=0;i<needJuzu.size();i++){
                QString each = needJuzu[i];
                if(each.contains("avg")){
                    QStringList localSplit = each.split('(');
                    QString value = localSplit.at(localSplit.size()-1); //grade)
                    value = value.left(value.size()-1);
                    m["avg"] = value;
                }
            }
            //max
            for(int i=0;i<needJuzu.size();i++){
                QString each = needJuzu[i];
                if(each.contains("max")){
                    QStringList localSplit = each.split('(');
                    QString value = localSplit.at(localSplit.size()-1); //grade)
                    value = value.left(value.size()-1);
                    m["max"] = value;
                }
            }
            //min
            for(int i=0;i<needJuzu.size();i++){
                QString each = needJuzu[i];
                if(each.contains("min")){
                    QStringList localSplit = each.split('(');
                    QString value = localSplit.at(localSplit.size()-1); //grade)
                    value = value.left(value.size()-1);
                    m["min"] = value;
                }
            }
            //count
            for(int i=0;i<needJuzu.size();i++){
                QString each = needJuzu[i];
                if(each.contains("count")){
                    QStringList localSplit = each.split('(');
                    QString value = localSplit.at(localSplit.size()-1); //grade)
                    value = value.left(value.size()-1);
                    m["count"] = value;
                }
            }
            return selectGroup(content,m,result);
        }


    }

    return result;
}
//用于条件判断
bool sqlAnalyze::ifSatisfyCondition(RecordEntity record,vector<QString> condition){
    QString ziduan = condition.at(0);
    QString fuhao = condition.at(1);
    QString value = condition.at(2);
    if(fuhao.compare("<")==0){
        QString realValue = record.getValue(ziduan);
        if(realValue.contains(".")){
            double doubleValue = realValue.toDouble();
            if(doubleValue < value.toDouble()){
                return true;
            }
            return false;
        }else{
            if(record.getValue(ziduan).compare(value)<0){
                return true;
            }
            return false;
        }
    }else if(fuhao.compare("=")==0){
        if(record.getValue(ziduan).compare(value)==0){
            return true;
        }
        return false;
    }else{
        QString realValue = record.getValue(ziduan);
        if(realValue.contains(".")){
            double doubleValue = realValue.toDouble();
            if(doubleValue > value.toDouble()){
                return true;
            }
            return false;
        }else{
            if(record.getValue(ziduan).compare(value)>0){
                return true;
            }
            return false;
        }
    }
}

//用于处理最后结果
vector<RecordEntity> sqlAnalyze::dealResult(vector<QString>needView,vector<RecordEntity>result,int size){
    //1.处理一下需要展示的字段
    int ifShowAll = 0;
    for(int i=0;i<needView.size();i++){
        if(needView.at(i).compare("*")==0){
            ifShowAll = 1;
            needView.clear();
        }
    }

    if(ifShowAll){
        RecordEntity record = result.at(size-1);
        map<QString, QString> mp = record.getValueMap();
        map<QString, QString>::iterator iter;
        for(iter = mp.begin();iter != mp.end();iter ++)
        {
            QString field = iter->first;
            needView.push_back(field);
        }
    }else{
        RecordEntity record = result.at(size-1);
        map<QString, QString> valueMap = record.getValueMap();
        for(int i=0;i<needView.size();i++){
            QString field = needView.at(i);
            if(valueMap.find(field)==valueMap.end()){
                RecordEntity status;
                status.SetValue("status","401"); //不存在字段
                result.push_back(status);
                return result;
            }
        }
    }
    //2.加入状态信息和需要展示的字段
    RecordEntity recordView;
    for(int i=0;i<needView.size();i++){
        recordView.SetValue(needView.at(i),"ok");
    }
    result.push_back(recordView);
    RecordEntity status;
    status.SetValue("status","200"); //成功
    result.push_back(status);
    return result;
}


vector<RecordEntity> sqlAnalyze::selectSqlsum(QString dbname, QString input){
    input = input.simplified(); //去除换行符和前后空格
       QStringList tem = input.split(' ');
       QString group = tem.last(); // 按什么字段分组
       regex selectOrRegex(" *select +[a-zA-Z0-9]* *( *, *sum\\([a-zA-Z0-9]*\\))?( *, *avg\\([a-zA-Z0-9]*\\))?( *, *max\\([a-zA-Z0-9]*\\))?( *, *min\\([a-zA-Z0-9]*\\))?( *, *count\\([a-zA-Z0-9]*\\))? +from +[a-zA-Z0-9]+( +where +[^ ]+ *= *[^ ]+( +or +[^ ]+ *= *[^ ]+)*)? +Group +By +[a-zA-Z0-9]+ *");
       regex selectAndRegex(" *select +[a-zA-Z0-9]* *( *, *sum\\([a-zA-Z0-9]*\\))?( *, *avg\\([a-zA-Z0-9]*\\))?( *, *max\\([a-zA-Z0-9]*\\))?( *, *min\\([a-zA-Z0-9]*\\))?( *, *count\\([a-zA-Z0-9]*\\))? +from +[a-zA-Z0-9]+( +where +[^ ]+ *= *[^ ]+( +and +[^ ]+ *= *[^ ]+)*)? +Group +By +[a-zA-Z0-9]+ *");
       vector<RecordEntity> result;
       map<QString,QString> groupView;
       if(regex_match(input.toStdString(),selectAndRegex)||regex_match(input.toStdString(),selectOrRegex)){
           // select (sno) , sum(grade) from student where ... Group By sno  //
           if(input.contains("where")){
               vector<vector<QString>> conditions;  // 储存具体条件
               vector<QString> needView;   // 具体字段
               //提取条件
               int selectL = input.indexOf("select");
               int formL = input.indexOf("from");
               int groupL = input.indexOf("Group");
               QString viewContent = input.mid(selectL+6,formL-selectL-6);  //将select和from之间的字段提取出来
               viewContent = viewContent.simplified();
               viewContent = viewContent.remove(QRegularExpression("\\s")); //去除空格 aaa,sum(...),avg(...),...
               QStringList views= viewContent.split(','); // views : aaa sum(...) avg() ... 或 sum(...) avg() ...
               //2.解析表名
               int whereL = input.indexOf("where");
               QString tableContent = input.mid(formL+4,whereL-formL-4);
               QString tableName = tableContent.simplified();
               // 3.判断表存在性
               QString path = DB_CREATE_PATH;
               path.append(dbname + "//" + tableName);
               if(!FileUtil().ifExistFolder(path)){
                   RecordEntity status;
                   status.SetValue("status","402"); //不存在表
                   result.push_back(status);
                   return result;
               }
               // select sno,sum(grade),max(grade),avg(tall)

               //4.获取用户要展示的字段
               vector<FieldEntity> fieldList = FieldService().getFieldList(databaseName,tableName);  // 获取字段信息
               for(int i=0;i<views.length();i++){
                   QString viewtem = views.at(i);
                   int leftL = viewtem.indexOf('(');
                   int rightL = viewtem.indexOf(')');
                   if(leftL>0){
                       QString yem = viewtem.mid(0,leftL);
                       viewtem = viewtem.mid(leftL+1,rightL-leftL-1); // 得到字段
                       groupView[yem] = viewtem;
                       needView.push_back(viewtem);

                   }
                   else {
                       needView.push_back(viewtem);
                   }
               }
               // 得到所有字段在needView(vector<QString>)里，所有的剧组函数信息在groupView(map<QString,QString>)里

               // 5 判断字段存在
               for(int i=0;i<needView.size();i++){
                   QString fieldView = needView.at(i);
                   int flag = 0;
                   for(int j=0;j<fieldList.size();j++){
                       FieldEntity field = fieldList.at(j);
                       if(field.getName().compare(fieldView)==0){
                           if(fieldView.compare(group)==0){ // 分组的字段
                               flag = 1;
                               break;
                           }
                           else if((int)(field.getType())==1||(int)(field.getType())==3){ // 剧组函数中的字段是整形或者浮点型
                               flag = 1;
                               break;
                           }
                       }
                   }
                   if(!flag){
                       RecordEntity status;
                       status.SetValue("status","401"); //不存在字段或字段非法
                       result.push_back(status);
                       return result;
                   }
               }

               // 判断字段合法

               //6解析and部分
               if(input.contains("and")){
                   QString andContent = input.mid(whereL+5,groupL-whereL-5); //w if g
                   andContent = andContent.simplified();
                   andContent = andContent.remove(QRegularExpression("\\s")); //去除空格 x=1andy=2andc=6
                   QStringList andList = andContent.split("and");//[x=1,y=2,c=6]
                   for(int i=0;i<andList.size();i++){
                       QStringList tempList = andList[i].split('=');
                       //对字段名进行检查
                       int flag = 0;
                       for(int j=0;j<fieldList.size();j++){
                           FieldEntity field = fieldList.at(j);
                           if(field.getName().compare(tempList[0])==0){
                               flag = 1;
                               break;
                           }
                       }
                       if(!flag){
                           RecordEntity status;
                           status.SetValue("status","401"); //不存在字段
                           result.push_back(status);
                           return result;
                       }
                       vector<QString> temp;
                       temp.push_back(tempList[0]);
                       temp.push_back(tempList[1]);
                       conditions.push_back(temp);
                   }
                   //所有解析完成调用方法

                   vector<RecordEntity> recordList = RecordService().getRecordList(dbname,tableName);
                   for(int i=0;i<recordList.size();i++){
                       RecordEntity record = recordList.at(i);
                       int flag = 0;
                       for(int j=0;j<conditions.size();j++){
                           if(record.getValue(conditions.at(j).at(0)).compare(conditions.at(j).at(1))!=0){
                               flag =1;
                               break;
                           }
                       }
                       if(!flag){
                           result.push_back(record);
                       }
                   }
                   // -------   结果取出
               }// and解析完成
               if(input.contains("or")){
                   QString andContent = input.mid(whereL+5,groupL-whereL-5); //w if g
                   andContent = andContent.simplified();
                   andContent = andContent.remove(QRegularExpression("\\s")); //去除空格 x=1andy=2andc=6
                   QStringList andList = andContent.split("or");//[x=1,y=2,c=6]
                   for(int i=0;i<andList.size();i++){
                       QStringList tempList = andList[i].split('=');
                       //对字段名进行检查
                       int flag = 0;
                       for(int j=0;j<fieldList.size();j++){
                           FieldEntity field = fieldList.at(j);
                           if(field.getName().compare(tempList[0])==0){
                               flag = 1;
                               break;
                           }
                       }
                       if(!flag){
                           RecordEntity status;
                           status.SetValue("status","401"); //不存在字段
                           result.push_back(status);
                           return result;
                       }
                       vector<QString> temp;
                       temp.push_back(tempList[0]);
                       temp.push_back(tempList[1]);
                       conditions.push_back(temp);
                   }
                   //所有解析完成调用方法

                   vector<RecordEntity> recordList = RecordService().getRecordList(dbname,tableName);
                   for(int i=0;i<recordList.size();i++){
                       RecordEntity record = recordList.at(i);
                       int flag = 0;
                       for(int j=0;j<conditions.size();j++){
                           if(record.getValue(conditions.at(j).at(0)).compare(conditions.at(j).at(1))==0){
                               flag =1;
                               break;
                           }
                       }
                       if(flag){
                           result.push_back(record);
                       }
                   }
                   // -------   结果取出
               }// or 解析完成*/

           }
           if(!input.contains("where")) { // 不包含where
               vector<vector<QString>> conditions;  // 储存具体条件
               vector<QString> needView;   // 具体字段
               //提取条件
               int selectL = input.indexOf("select");
               int formL = input.indexOf("from");
               int groupL = input.indexOf("Group");
               QString viewContent = input.mid(selectL+6,formL-selectL-6);  //将select和from之间的字段提取出来
               viewContent = viewContent.simplified();
               viewContent = viewContent.remove(QRegularExpression("\\s")); //去除空格 aaa,sum(...),avg(...),...
               QStringList views= viewContent.split(','); // views : aaa sum(...) avg() ... 或 sum(...) avg() ...
               //2.解析表名
               QString tableContent = input.mid(formL+4,groupL-formL-4);
               QString tableName = tableContent.simplified();
               // 3.判断表存在性
               QString path = DB_CREATE_PATH;
               path.append(dbname + "//" + tableName);
               if(!FileUtil().ifExistFolder(path)){
                   RecordEntity status;
                   status.SetValue("status","402"); //不存在表
                   result.push_back(status);
                   return result;
               }
               // select sno,sum(grade),max(grade),avg(tall)

               //4.判断用户要展示的字段
               vector<FieldEntity> fieldList = FieldService().getFieldList(databaseName,tableName);  // 获取字段信息
               for(int i=0;i<views.length();i++){
                   QString viewtem = views.at(i);
                   int leftL = viewtem.indexOf('(');
                   int rightL = viewtem.indexOf(')');
                   if(leftL>0){
                       QString yem = viewtem.mid(0,leftL);
                       viewtem = viewtem.mid(leftL+1,rightL-leftL-1); // 得到字段
                       groupView.insert(pair<QString, QString>(yem, viewtem));
                       needView.push_back(viewtem);  // 格式为 ("sum","grade")
                   }
                   else {
                       needView.push_back(viewtem);
                   }
               }
               // 得到所有字段在needView(vector<QString>)里，所有的剧组函数信息在groupView(map<QString,QString>)里

               // 5 判断字段合法
               for(int i=0;i<needView.size();i++){
                   QString fieldView = needView.at(i);
                   int flag = 0;
                   for(int j=0;j<fieldList.size();j++){
                       FieldEntity field = fieldList.at(j);
                       if(field.getName().compare(fieldView)==0){
                           if(fieldView.compare(group)==0){ // 分组的字段
                               flag = 1;
                               break;
                           }
                           else if(field.getType()==1||field.getType()==5){ // 剧组函数中的字段是整形或者浮点型
                               flag = 1;
                               break;
                           }
                       }
                   }
                   if(!flag){
                       RecordEntity status;
                       status.SetValue("status","401"); //不存在字段
                       result.push_back(status);
                       return result;
                   }
               }
               //所有解析完成调用方法

               vector<RecordEntity> recordList = RecordService().getRecordList(dbname,tableName);
               for(int i=0;i<recordList.size();i++){
                   RecordEntity record = recordList.at(i);
                   result.push_back(record);
               }
               // -------   结果取出
           }
           map<QString, QString>::iterator iter;
               for(iter = groupView.begin(); iter !=groupView.end(); iter++){
                   cout<<iter->first.toStdString()<<"+"<<iter->second.toStdString()<<"----";
               }
           return selectGroup(group,groupView,result);   // result group groupView
           //return result;select sno,sum(grade),max(credit) from sc Group By sno
       }
       else {
           RecordEntity statu;
           statu.SetValue("status","402"); //语句不合法
           result.push_back(statu);
           return result;
       }
}

vector<RecordEntity> sqlAnalyze::selectGroup(QString group, map<QString, QString> m, vector<RecordEntity> recordList){
    set<QString> groupset;  // recordList中的group集合
    set<QString> viewset; // map中的字段集合
    vector<RecordEntity> result; //返回的结果集
    vector<vector<RecordEntity>>  groupR;
    for(int i=0;i<recordList.size();i++){
        map<QString,QString> s = recordList.at(i).getValueMap();
        groupset.insert(s[group]);

    }// 按照group分组实现
    set<QString>::iterator it;
    int i=0;
    for(it=groupset.begin();it!=groupset.end();it++){
        vector<RecordEntity> e;
        for(int j=0;j<recordList.size();j++){
            map<QString,QString> s = recordList.at(j).getValueMap();
            if(s[group].compare(*it)==0){
                e.push_back(recordList.at(j));
            }
        }
        groupR.push_back(e);
    }
    // 分组完成
    map<QString, QString>::iterator iter;
    for(iter = m.begin(); iter != m.end(); iter++){
        viewset.insert(iter->second);
    }
    // 按map中的字段分组完成


    for(i=0;i<groupR.size();i++){
        RecordEntity tem;
        vector<RecordEntity> resu = groupR.at(i); // 第i组
        map<QString,QString> sem=resu.at(0).getValueMap();
        tem.SetValue(group,sem[group]); // 先将分组字段值填充
        for(it=viewset.begin();it!=viewset.end();it++){ // 第i个字段
            double min=10000,max=0,sum=0;
            int count = resu.size();
            for(int j=0;j<resu.size();j++){
                map<QString,QString> s=resu.at(j).getValueMap();
                double t = s[(*it)].toDouble();
                if(t<min) min = t;
                if(t>max) max = t;
                sum+=t;
            }
            for(iter = m.begin(); iter != m.end(); iter++){
                if(iter->second.compare(*it)==0){
                    if(iter->first=="sum"){
                        QString gs = "sum";
                        gs.append('(');gs.append(*it);gs.append(')');
                        tem.SetValue(gs,QString::number(sum));
                    }
                    else if(iter->first=="count"){
                        QString gs = "count";
                        gs.append('(');gs.append(*it);gs.append(')');
                        tem.SetValue(gs,QString::number(count));
                    }
                    else if(iter->first=="avg"){
                        QString gs = "avg";
                        gs.append('(');gs.append(*it);gs.append(')');
                        double t = sum/count;
                        tem.SetValue(gs,QString::number(t));
                    }
                    else if(iter->first=="max"){
                        QString gs = "max";
                        gs.append('(');gs.append(*it);gs.append(')');
                        tem.SetValue(gs,QString::number(max));
                    }
                    else if(iter->first=="min"){
                        QString gs = "min";
                        gs.append('(');gs.append(*it);gs.append(')');
                        tem.SetValue(gs,QString::number(min));
                    }
                }
            }
        }
        result.push_back(tem);
    }
    RecordEntity g;
    g.SetValue(group,group);
    for(iter = m.begin(); iter != m.end(); iter++){
            QString ss = iter->first;
            ss.append('('+iter->second+')');
            g.SetValue(ss,iter->second);
    }
    result.push_back(g);
    RecordEntity status;
    status.SetValue("status","200"); //成功
    result.push_back(status);
    return result;
}

int sqlAnalyze::operatorInAlterTable(QString input)
{
    string inputString = std::string(input.toLocal8Bit());// 输入字符串的string类型

    /* 第一种情况，匹配添加新列的语句 */
    regex pattern1(" *[Aa][Ll][Tt][Ee][Rr] +[Tt][Aa][Bb][Ll][Ee] +([^ ]+) +[Aa][Dd][Dd] +([^ ]+) +([^ ]+) *(.*)");
    smatch result1;// 若匹配成功，result1[0]表示匹配的完整结果，result1[1]表示 表名 ，result1[2]表示 字段名 ， result1[3]表示 字段类型，result1[4]表示 约束条件(约束条件也可不存在)

    /* 第二种情况，匹配改变列的属性的语句 */
    regex pattern2(" *[Aa][Ll][Tt][Ee][Rr] +[Tt][Aa][Bb][Ll][Ee] +([^ ]+) +[Mm][Oo][Dd][Ii][Ff][Yy] +([^ ]+) +([^ ]+) *(.*)");
    smatch result2;// 若匹配成功，result2[0]表示匹配的完整结果，result2[1]表示 表名 ，result2[2]表示 字段名 ， result2[3]表示 字段类型，result2[4]表示 约束条件(约束条件也可不存在)

    /* 第三种情况，匹配删除现有的列的语句 */
    regex pattern3(" *[Aa][Ll][Tt][Ee][Rr] +[Tt][Aa][Bb][Ll][Ee] +([^ ]+) +[Dd][Rr][Oo][Pp] +[Cc][Oo][Ll][Uu][Mm][Nn] +([^ ]+) *;? *");
    smatch result3;// 若匹配成功，result3[0]表示匹配的完整结果，result3[1]表示 表名 ，result3[2]表示 字段名

    /* 第四种情况，匹配重命名列的语句 */
    regex pattern4(" *[Aa][Ll][Tt][Ee][Rr] +[Tt][Aa][Bb][Ll][Ee] +([^ ]+) +[Rr][Ee][Nn][Aa][Mm][Ee] +[Cc][Oo][Ll][Uu][Mm][Nn] +([^ ]+) +[Tt][Oo] +([^ ]+) *;? *");
    smatch result4;// 若匹配成功，result4[0]表示匹配的完整结果，result4[1]表示 表名 ，result4[2]表示 原字段名，result4[3]表示 修改后的字段名

    /* 第五种情况，匹配重命名表的语句 */
    regex pattern5(" *[Aa][Ll][Tt][Ee][Rr] +[Tt][Aa][Bb][Ll][Ee] +([^ ]+) +[Rr][Ee][Nn][Aa][Mm][Ee] +[Tt][Oo] +([^ ]+) *;? *");
    smatch result5;// 若匹配成功，result5[0]表示匹配的完整结果，result5[1]表示 原表名 ，result3[2]表示 新表名

    /* 尝试匹配 */
    if (regex_match(inputString, result1, pattern1))
    {
        cout << "1" << endl;
        cout << "tablename:" << result1[1] << '\n' << "fieldname:" << result1[2] << '\n' << "fieldtype:" << result1[3] << '\n' << "constraint:" << result1[4] << endl;

        FieldEntity field =  FieldEntity();// 存储字段相关的信息

        /* 设置字段名 */
        field.setName(QString::fromStdString(result1[2].str()));

        /* 设置字段类型,如果新字段的类型是varchar(n)类型,则进行一些额外的设置 */
        regex patternVarchar("[Vv][Aa][Rr][Cc][Hh][Aa][Rr][(]([0-9]+)[)]");
        smatch resultVarchar;// 如果匹配成功，resultVarchar[1]返回varchar类型的长度
        string fieldType = result1[3].str();// 字段类型的string

        if (regex_match(fieldType, resultVarchar, patternVarchar))
        {
            int length = atoi(resultVarchar[1].str().c_str());// varchar类型的长度
            field.setLength(length);// 设置varchar长度
            field.setType(4);// 设置字段类型为varchar
        }
        else if (result1[3].str().compare("int") == 0)
            field.setType(1);
        else if (result1[3].str().compare("bool") == 0)
            field.setType(2);
        else if (result1[3].str().compare("time") == 0)
            field.setType(3);
        else if (result1[3].str().compare("double") == 0)
            field.setType(5);
        else
            return 0;

        /* 如果有约束条件,匹配约束条件 */
        string constraintString = result1[4].str();// 约束条件的string类型
        if (constraintString != "")
        {
            smatch resultConstraint;// 接收约束条件的匹配结果，如果匹配成功则resultConstraint[1]返回默认值
            regex patternConstraint1("[Uu][Nn][Ii][Qq][Uu][Ee] +[Dd][Ee][Ff][Aa][Uu][Ll][Tt] +(.+)");// 匹配约束条件:unique default x
            regex patternConstraint2("[Nn][Oo][Tt] +[Nn][Uu][Ll][Ll] +[Dd][Ee][Ff][Aa][Uu][Ll][Tt] +(.+)");// 匹配约束条件:not null default x
            regex patternConstraint3("[Uu][Nn][Ii][Qq][Uu][Ee] +[Nn][Oo][Tt] +[Nn][Uu][Ll][Ll] +[Dd][Ee][Ff][Aa][Uu][Ll][Tt] +(.+)");// 匹配约束条件:unique not null default x

            if (result1[4].str().compare("primary key") == 0)
            {
                cout << "constraint is primary key" << endl;
                field.setIsPK(1);
                field.setIsUnique(1);
                field.setIsNotNUll(1);
            }
            else if (result1[4].str().compare("unique") == 0)
            {
                cout << "constraint is unique" << endl;
                field.setIsPK(0);
                field.setIsUnique(1);
                field.setIsNotNUll(0);
            }
            else if (result1[4].str().compare("not null") == 0)
            {
                cout << "constraint is not null" << endl;
                field.setIsPK(0);
                field.setIsUnique(0);
                field.setIsNotNUll(1);
            }
            else if (result1[4].str().compare("unique not null") == 0)
            {
                cout << "constraint is unique not null" << endl;
                field.setIsPK(0);
                field.setIsUnique(1);
                field.setIsNotNUll(1);
            }
            else if (regex_match(constraintString, resultConstraint, patternConstraint1))
            {
                cout << "constraint is unique default x" << endl;
                field.setIsPK(0);
                field.setIsUnique(1);
                field.setIsNotNUll(0);
                field.setDefaultValue(QString::fromStdString(resultConstraint[1].str()));
            }
            else if (regex_match(constraintString, resultConstraint, patternConstraint2))
            {
                cout << "constraint is not null default x" << endl;
                field.setIsPK(0);
                field.setIsUnique(0);
                field.setIsNotNUll(1);
                field.setDefaultValue(QString::fromStdString(resultConstraint[1].str()));
            }
            else if (regex_match(constraintString, resultConstraint, patternConstraint3))
            {
                cout << "constraint is unique not null default x" << endl;
                field.setIsPK(0);
                field.setIsUnique(1);
                field.setIsNotNUll(1);
                field.setDefaultValue(QString::fromStdString(resultConstraint[1].str()));
            }
            else
                return 0;
        }

        /* 插入字段 */
        return FieldService().insertField(databaseName, QString::fromStdString(result1[1].str()), field);
    }
    else if (regex_match(inputString, result2, pattern2))
    {
        cout << "2" << endl;
        cout << "tablename:" << result2[1] << '\n' << "fieldname:" << result2[2] << '\n' << "fieldtype:" << result2[3] << '\n' << "constraint:" << result2[4] << endl;

        FieldEntity field =  FieldEntity();// 存储字段相关的信息

        /* 设置字段名 */
        field.setName(QString::fromStdString(result2[2].str()));

        /* 设置字段类型,如果新字段的类型是varchar(n)类型,则进行一些额外的设置 */
        regex patternVarchar("[Vv][Aa][Rr][Cc][Hh][Aa][Rr][(]([0-9]+)[)]");
        smatch resultVarchar;// 如果匹配成功，resultVarchar[1]返回varchar类型的长度
        string fieldType = result2[3].str();// 字段类型的string

        if (regex_match(fieldType, resultVarchar, patternVarchar))
        {
            int length = atoi(resultVarchar[1].str().c_str());// varchar类型的长度
            field.setLength(length);// 设置varchar长度
            field.setType(4);// 设置字段类型为varchar
        }
        else if (result2[3].str().compare("int") == 0)
            field.setType(1);
        else if (result2[3].str().compare("bool") == 0)
            field.setType(2);
        else if (result2[3].str().compare("time") == 0)
            field.setType(3);
        else if (result2[3].str().compare("double") == 0)
            field.setType(5);
        else
            return 0;

        /* 如果有约束条件,匹配约束条件 */
        string constraintString = result2[4].str();// 约束条件的string类型
        if (constraintString != "")
        {
            smatch resultConstraint;// 接收约束条件的匹配结果，如果匹配成功则resultConstraint[1]返回默认值
            regex patternConstraint1("[Uu][Nn][Ii][Qq][Uu][Ee] +[Dd][Ee][Ff][Aa][Uu][Ll][Tt] +(.+)");// 匹配约束条件:unique default x
            regex patternConstraint2("[Nn][Oo][Tt] +[Nn][Uu][Ll][Ll] +[Dd][Ee][Ff][Aa][Uu][Ll][Tt] +(.+)");// 匹配约束条件:not null default x
            regex patternConstraint3("[Uu][Nn][Ii][Qq][Uu][Ee] +[Nn][Oo][Tt] +[Nn][Uu][Ll][Ll] +[Dd][Ee][Ff][Aa][Uu][Ll][Tt] +(.+)");// 匹配约束条件:unique not null default x

            if (result2[4].str().compare("primary key") == 0)
            {
                cout << "constraint is primary key" << endl;
                field.setIsPK(1);
                field.setIsUnique(1);
                field.setIsNotNUll(1);
            }
            else if (result2[4].str().compare("unique") == 0)
            {
                cout << "constraint is unique" << endl;
                field.setIsPK(0);
                field.setIsUnique(1);
                field.setIsNotNUll(0);
            }
            else if (result2[4].str().compare("not null") == 0)
            {
                cout << "constraint is not null" << endl;
                field.setIsPK(0);
                field.setIsUnique(0);
                field.setIsNotNUll(1);
            }
            else if (result2[4].str().compare("unique not null") == 0)
            {
                cout << "constraint is unique not null" << endl;
                field.setIsPK(0);
                field.setIsUnique(1);
                field.setIsNotNUll(1);
            }
            else if (regex_match(constraintString, resultConstraint, patternConstraint1))
            {
                cout << "constraint is unique default x" << endl;
                field.setIsPK(0);
                field.setIsUnique(1);
                field.setIsNotNUll(0);
                field.setDefaultValue(QString::fromStdString(resultConstraint[1].str()));
            }
            else if (regex_match(constraintString, resultConstraint, patternConstraint2))
            {
                cout << "constraint is not null default x" << endl;
                field.setIsPK(0);
                field.setIsUnique(0);
                field.setIsNotNUll(1);
                field.setDefaultValue(QString::fromStdString(resultConstraint[1].str()));
            }
            else if (regex_match(constraintString, resultConstraint, patternConstraint3))
            {
                cout << "constraint is unique not null default x" << endl;
                field.setIsPK(0);
                field.setIsUnique(1);
                field.setIsNotNUll(1);
                field.setDefaultValue(QString::fromStdString(resultConstraint[1].str()));
            }
            else
                return 0;
        }

        /* 更改字段 */
        return FieldService().modifyField(databaseName, QString::fromStdString(result2[1].str()), QString::fromStdString(result2[2].str()), field);
    }
    else if (regex_match(inputString, result3, pattern3))
    {
        cout << "3" << endl;
        cout << "tablename:" << result3[1] << '\n' << "fieldname:" << result3[2] << endl;

        /* 删除字段 */
        return FieldService().deleteField(databaseName, QString::fromStdString(result3[1].str()), QString::fromStdString(result3[2].str()));
    }
    else if (regex_match(inputString, result4, pattern4))
    {
        cout << "4" << endl;
        cout << "tablename:" << result4[1] << '\n' << "oldFieldName:" << result4[2] << '\n' << "newFieldName:" << result4[3] << endl;

        /* 获得表中所有字段信息 */
        vector<FieldEntity> fieldList = FieldService().getFieldList(databaseName, QString::fromStdString(result4[1].str()));

        /* 找到需要修改的字段的信息,赋值给field */
        FieldEntity field;
        for (unsigned int i = 0 ; i < fieldList.size() ; i++)
            if (fieldList[i].getName().compare(QString::fromStdString(result4[2])) == 0)
                field = fieldList[i];

        /* 将字段的名字改为新名字 */
        field.setName(QString::fromStdString(result4[3].str()));

        /* 以修改字段的方式修改字段名 */
        return FieldService().modifyField(databaseName, QString::fromStdString(result4[1].str()), QString::fromStdString(result4[2].str()), field);
    }
    else if (regex_match(inputString, result5, pattern5))
    {
        cout << "5" << endl;
        cout << "oldtablename:" << result5[1] << '\n' << "newtablename:" << result5[2];

        TableService table;
        table.setDatabaseName(databaseName);
        return table.ModifyTableName(QString::fromStdString(result5[1].str()), QString::fromStdString(result5[2].str()));
    }
    else
    {
        cout << "Error!" << endl;
        return 0;
    }

    return 0;
}
