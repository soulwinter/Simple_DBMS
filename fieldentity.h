#ifndef FIELDENTITY_H
#define FIELDENTITY_H
#include <QString>
using namespace std;
class FieldEntity
{
private:
    int id;
    QString name;
    int type;
    int length;
    double max = 10000000;
    double min =-10000000;
    QString defaultValue="null";//null为没有默认值
    int isNotNUll = 1;
    int isUnique = 0;
    int isPK = 0;
    QString comment ="";
    int isSuoYin = 0;

public:
    FieldEntity();
    //get
    int getId() const;
    QString getName() const;
    int getType() const;
    int getLength() const;
    double getMax() const;
    double getMin() const;
    QString getDefaultValue() const;
    int getIsNotNUll() const;
    int getIsUnique() const;
    int getIsPK() const;
    QString getComment() const;
    //set
    void setId(int value);
    void setName(const QString& value);
    void setType(int value);
    void setLength(int value);
    void setMax(double value);
    void setMin(double value);
    void setDefaultValue(const QString& value);
    void setIsNotNUll(int value);
    void setIsUnique(int value);
    void setIsPK(int value);
    void setComment(const QString& value);
    int getIsSuoYin() const;
    void setIsSuoYin(int value);
};

#endif // FIELDENTITY_H
