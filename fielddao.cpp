#include "fielddao.h"

FieldDao::FieldDao(QString dbName, QString systemPath, QString tableName)
{
    this->filePath = systemPath + "//" + dbName + "//" + tableName;
}

int FieldDao::insertField(QString dbName, QString tableName, FieldEntity fieldEntity) {
    QString path = filePath + "//" + tableName + ".tdf";//储存字段信息的文件路径
    if (!FileUtil().ifExistFile(path)) {
        return 308;// TDF_NOT_EXISTS 308
    }
    int FieldNumber = FileUtil().getCounter(path);
    FileUtil().replaceContent(path, QString::number(FieldNumber) + "#" + dbName+"#"+tableName, QString::number(FieldNumber + 1) + "#" + dbName+"#" + tableName);
    FileUtil().writeAtTail(path,
        QString::number(FieldNumber + 1) + "#"  +
        fieldEntity.getName() + "#" +
        QString::number(fieldEntity.getType()) + "#" +
        QString::number(fieldEntity.getLength()) + "#" +
        QString::number(fieldEntity.getMax()) + "#" +
        QString::number(fieldEntity.getMin()) + "#" +
        fieldEntity.getDefaultValue() + "#" +
        QString::number(fieldEntity.getIsPK()) + "#" +
        QString::number(fieldEntity.getIsNotNUll()) + "#" +
        QString::number(fieldEntity.getIsUnique()) + "#" +
        fieldEntity.getComment());


    //向trd文件插入内容
    QString path2 = filePath + "//" + tableName + ".trd";
    if (!FileUtil().ifExistFile(path2)) {
        return 309;// TRD_NOT_EXISTS 309
    }
    int dataNumber = FileUtil().getCounter(path2);
    QString blankSpace="null";
    for (int i = 2; i < dataNumber + 2; i++)
    {
        QString addContent;
        if (fieldEntity.getIsPK()==1)
        {
            if (fieldEntity.getType() == 1)
                 addContent = QString::number(i - 1);
            if (fieldEntity.getType() == 2)
                 addContent = "true";
            if (fieldEntity.getType() == 3)
                addContent = QString::number(2000 + i - 1);
                addContent = addContent.append("-01-01");
            if (fieldEntity.getType() == 4)
                addContent = QString::number(i - 1);
            if (fieldEntity.getType() == 5)
            {
                addContent = QString::number(i - 1);
                addContent +=".0";
            }
        }
        else if (!(fieldEntity.getDefaultValue() == blankSpace))
        {
            addContent = fieldEntity.getDefaultValue();
        }
        else if (fieldEntity.getIsNotNUll()==1 && fieldEntity.getDefaultValue() == blankSpace)
        {
            if (fieldEntity.getType() == 1)
                addContent = QString::number(i - 1);
            if (fieldEntity.getType() == 2)
                 addContent = "true";
            if (fieldEntity.getType() == 3)
                addContent = QString::number(i - 1);
                addContent = QString::number(2000 + i - 1);
                addContent = addContent.append("-01-01");
            if (fieldEntity.getType() == 4)
                addContent = QString::number(i - 1);
            if (fieldEntity.getType() == 5)
            {
                addContent = QString::number(i - 1);
                addContent =addContent+".0";
            }
        }
        else
        {
            addContent = "null";
        }
        QString oldContent= fileUtil.readLine(path2, i);
        QString newContent = oldContent + '#' + addContent;
        FileUtil().replaceContent(path2, oldContent, newContent);
    }
    return 1;
}
vector<FieldEntity> FieldDao::getFieldList(QString tableName) {
    QString path = filePath + "//" + tableName + ".tdf";
    int number = FileUtil().getCounter(path);
    vector<FieldEntity> list;
    for (int i = 2; i <= number + 1; i++)
    {
        QString lineRead = FileUtil().readLine(path, i);
        for (int j = 0; j < lineRead.length(); j++)
        {
            if (lineRead[j] == '#')
                lineRead[j] = ' ';
        }

        int id;//id
        QString name;//字段名
        int type;//字段类型
        int length;//字段长度
        double max;//最大值
        double min;//最小值
        QString defaultValue;//默认值
        int isPK;//是否主键
        int isNotNUll;//是否为非空
        int isUnique;//是否唯一
        QString comment;//注释

        QTextStream strStream(&lineRead);
        strStream >> id >> name >> type >> length >> max >> min >> defaultValue >> isPK >> isNotNUll >> isUnique >> comment;
        FieldEntity newField;
        newField.setId(id);
        newField.setName(name);
        newField.setType(type);
        newField.setLength(length);
        newField.setMax(max);
        newField.setMin(min);
        newField.setDefaultValue(defaultValue);
        newField.setIsPK(isPK);
        newField.setIsNotNUll(isNotNUll);
        newField.setIsUnique(isUnique);
        newField.setComment(comment);

        list.push_back(newField);
    }
    return list;
}

int FieldDao::deleteField(QString dbName, QString tableName, QString fieldName)
{
    QString path = filePath + "//" + tableName + ".tdf";//储存字段信息的文件路径
    if (!FileUtil().ifExistFile(path)) {
        return 308;// TDF_NOT_EXISTS 308
    }
    int line = fileUtil.getLineByName(path, fieldName);//找到删除字段的所在行数
    if(line <=0) return 310;

    int Line = line - 1;//所删除的字段的Id 底下修改trd文件时使用

    int FieldNumber = FileUtil().getCounter(path);
    FileUtil().replaceContent(path, QString::number(FieldNumber) + "#" + dbName + "#" + tableName, QString::number(FieldNumber - 1) + "#" + dbName + "#" + tableName);
    if (line == 0)
        return 310;//310->删除的字段名不存在
    fileUtil.deleteLine(path, line);

    //替换剩下字段的Id
    QString str; QString Id; int id; QString newStr;
    for (; line < FieldNumber+1; line++)
    {
        str = fileUtil.readLine(path, line);
        Id = str[0];
        id = Id.toInt();
        id--;
        newStr += QString::number(id);
        newStr += str.mid(1);
        fileUtil.replaceContent(path, str, newStr);
        newStr = "";
    }

    //修改trd文件
    QString path2 = filePath + "//" + tableName + ".trd";
    if (!FileUtil().ifExistFile(path2)) {
        return 309;// TRD_NOT_EXISTS 309
    }
    int dataNumber = FileUtil().getCounter(path2);
    QString str2; QStringList strList; QString newStr2; int j = 0;
    for (int i = 2; i < dataNumber + 2; i++)
    {
        str2 = fileUtil.readLine(path2, i);
        strList = str2.split("#");
        for (j = 0; j < Line; j++)
        {
            newStr2 += strList.at(j);
            if((j+1)!=FieldNumber)
            newStr2 += '#';
        }
        if (j == FieldNumber){}
        else
        {
            j++;
            for (; j < FieldNumber; j++)
            {
                newStr2 += strList.at(j);
                newStr2 += '#';
            }
            newStr2 += strList.at(j);
        }
        fileUtil.replaceContent(path2, str2, newStr2);
        newStr2 = "";
    }
    return 1;
}
//这个方法注意还需删除表记录中的对应值

int FieldDao::modifyField(QString tableName,QString oldFieldName, FieldEntity newfieldEntity)
{
    QString path = filePath + "//" + tableName + ".tdf";//储存字段信息的文件路径
    if (!FileUtil().ifExistFile(path)) {
        return 308;// TDF_NOT_EXISTS 308
    }
    int line = fileUtil.getLineByName(path, oldFieldName);//找到修改字段的所在行数
    QString str; QStringList strList; QString newStr;
    str = fileUtil.readLine(path, line);
    strList = str.split("#");
    newStr += strList.at(0);
    newStr += "#"; newStr += newfieldEntity.getName(); newStr += "#";
    newStr += QString::number(newfieldEntity.getType()); newStr += "#";
    newStr += QString::number(newfieldEntity.getLength()); newStr += "#";
    newStr += QString::number(newfieldEntity.getMax()); newStr += "#";
    newStr += QString::number(newfieldEntity.getMin()); newStr += "#";
    newStr += newfieldEntity.getDefaultValue(); newStr += "#";
    newStr += QString::number(newfieldEntity.getIsPK()); newStr += "#";
    newStr += QString::number(newfieldEntity.getIsNotNUll()); newStr += "#";
    newStr += QString::number(newfieldEntity.getIsUnique()); newStr += "#";
    newStr +=newfieldEntity.getComment();
    fileUtil.replaceContent(path, str, newStr);
    return 1;
}
