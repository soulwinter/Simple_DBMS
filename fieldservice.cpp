#include "fieldservice.h"
#include "recordservice.h"

FieldService::FieldService()
{

}

int FieldService::insertField(QString dbName,QString tableName,FieldEntity fieldEntity){
    FieldDao* fieldDao = new FieldDao(dbName,DB_CREATE_PATH,tableName);

    for(int i=0;i<getFieldList(dbName,tableName).size();i++)
    {
        if(fieldEntity.getName()==getFieldList(dbName,tableName)[i].getName())
            return 0;
    }

    return fieldDao->insertField(dbName,tableName,fieldEntity);
}
 //这个方法注意还需删除表记录中的对应值
int FieldService::deleteField(QString dbName,QString tableName,QString fieldName){
    FieldDao* fieldDao = new FieldDao(dbName,DB_CREATE_PATH,tableName);

    return fieldDao->deleteField(dbName,tableName,fieldName);
}

int FieldService::modifyField(QString dbName,QString tableName,QString oldFieldName,FieldEntity newfieldEntity){
    FieldDao* fieldDao = new FieldDao(dbName,DB_CREATE_PATH,tableName);

    if(oldFieldName.compare(newfieldEntity.getName())!=0){
        for(int i=0;i<getFieldList(dbName,tableName).size();i++)
        {
            if(newfieldEntity.getName()==getFieldList(dbName,tableName)[i].getName())
                return 0;
        }
    }
    //判断一下已有的数据是否满足约束条件
    //1.先把所有的记录取出来
    vector<RecordEntity> recordList = RecordService().getRecordList(dbName,tableName);
    //not null情况
    if(newfieldEntity.getIsNotNUll()==1){
        for(int i=0;i<recordList.size();i++){
            RecordEntity record = recordList.at(i);
            QString content = record.getValue(oldFieldName);
            if(content.compare("null")==0){
                return 10087; //修改字段类型错误：null
            }
        }
    }
    if(newfieldEntity.getIsPK()==1||newfieldEntity.getIsUnique()==1){
        map<QString,int> tempMap;
        for(int i=0;i<recordList.size();i++){
            RecordEntity record = recordList.at(i);
            QString content = record.getValue(oldFieldName);
            if(tempMap.find(content)==tempMap.end()){
                tempMap[content] = 1;
            }else{
                return 10090; //修改字段类型错误: PK or unique
            }
        }
    }
    //检查长度，最大最小值
    for(int i=0;i<recordList.size();i++){
        RecordEntity record = recordList.at(i);
        QString content = record.getValue(oldFieldName);
        if(newfieldEntity.getType()==4){
            //字符型才检查长度
            if(content.size()>newfieldEntity.getLength()){
                return 10088;//修改字段类型错误：length
            }
        }
        if(newfieldEntity.getType()==INT){
            int max = newfieldEntity.getMax();
            int min = newfieldEntity.getMin();
            if(content.toInt()<min || content.toInt()>max){
                 return 10089;//修改字段类型错误：max or min
            }
        }
        if(newfieldEntity.getType()==DOUBLE){
            double max = newfieldEntity.getMax();
            double min = newfieldEntity.getMin();
            if(content.toDouble()<min || content.toDouble()>max){
                 return 10089;//修改字段类型错误：max or min
            }
        }
    }

    return fieldDao->modifyField(tableName,oldFieldName,newfieldEntity);
}

vector<FieldEntity> FieldService::getFieldList(QString dbName,QString tableName){
     FieldDao* fieldDao = new FieldDao(dbName,DB_CREATE_PATH,tableName);
     return fieldDao->getFieldList(tableName);
}
