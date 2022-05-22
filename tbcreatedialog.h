#ifndef TBCREATEDIALOG_H
#define TBCREATEDIALOG_H

#include <QDialog>
#include "dbservice.h"
#include "tableservice.h"
#include "fielddialog.h"
#include "fieldentity.h"
#include <vector>

namespace Ui {
class TBCreateDialog;
}

class TBCreateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TBCreateDialog(QWidget *parent = 0,QString databaseName = "",QString tableName = "");
    ~TBCreateDialog();

    
    Ui::TBCreateDialog *getUi() const;
    
    vector<FieldEntity> getFields() const;
    void setFields(const vector<FieldEntity> &value);

    vector<FieldEntity> getViewfields() const;
    void setViewfields(const vector<FieldEntity> &value);

    vector<FieldEntity> getCopyViewfields() const;
    void setCopyViewfields(const vector<FieldEntity> &value);

    QString getTableName() const;
    void setTableName(const QString &value);

private:
    Ui::TBCreateDialog *ui;
    QString databaseName;

    vector<FieldEntity> fields;
    vector<FieldEntity> viewfields;
    vector<FieldEntity> copyViewfields; //表结构的拷贝，用户做的修改都在这个上面做，最后用于更新


    QString tableName;
    
    void init();
    void showStruct();
    void addItemToWidget(FieldEntity* field);

private slots:
    int save();
    void addField();
    void flushAddField(FieldEntity* field);
    void setFieldEdit(int row,int col);
    int deleteField();
    int modifyField();
    void flushMofifyField(FieldEntity* field);

signals:
    void sendData(QString dbName,QString tableName);
};

#endif // TBCREATEDIALOG_H
