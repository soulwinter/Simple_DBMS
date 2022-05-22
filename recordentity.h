#ifndef RECORDENTITY_H
#define RECORDENTITY_H
#include <map>
#include <QString>
using namespace std;
class RecordEntity
{
private:
    int id;
    map<QString,QString> valueMap;
 
public:
    RecordEntity();
    //get
    int getId() const;
    QString getValue(QString field);
    map<QString, QString> getValueMap() const;
    // age name
    void setId(int value);
    void SetValue(QString field,QString value);
    // SetValue
};

#endif // RECORDENTITY_H
