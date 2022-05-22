#include "fieldentity.h"

int FieldEntity::getId() const
{
    return id;
}

void FieldEntity::setId(int value)
{
    id = value;
}


QString FieldEntity::getName() const
{
    return name;
}

int FieldEntity::getType() const
{
    return type;
}

int FieldEntity::getLength() const
{
    return length;
}

double FieldEntity::getMax() const
{
    return max;
}

double FieldEntity::getMin() const
{
    return min;
}

QString FieldEntity::getDefaultValue() const
{
    return defaultValue;
}

int FieldEntity::getIsNotNUll() const
{
    return isNotNUll;
}

int FieldEntity::getIsUnique() const
{
    return isUnique;
}

int FieldEntity::getIsPK() const
{
    return isPK;
}

QString FieldEntity::getComment() const
{
    return comment;
}


void FieldEntity::setName(const QString& value)
{
    name = value;
}


void FieldEntity::setType(int value)
{
    type = value;
}

void FieldEntity::setLength(int value)
{
    length = value;
}

void FieldEntity::setMax(double value)
{
    max = value;
}

void FieldEntity::setMin(double value)
{
    min = value;
}

void FieldEntity::setDefaultValue(const QString& value)
{
    defaultValue = value;
}

void FieldEntity::setIsNotNUll(int value)
{
    isNotNUll = value;
}

void FieldEntity::setIsUnique(int value)
{
    isUnique = value;
}

void FieldEntity::setIsPK(int value)
{
    isPK = value;
}

void FieldEntity::setComment(const QString& value)
{
    comment = value;
}

int FieldEntity::getIsSuoYin() const
{
    return isSuoYin;
}

void FieldEntity::setIsSuoYin(int value)
{
    isSuoYin = value;
}

FieldEntity::FieldEntity()
{

}
