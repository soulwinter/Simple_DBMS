#ifndef FIELDDIALOG_H
#define FIELDDIALOG_H

#include <QDialog>
#include "fieldentity.h"
#include "fileutil.h"
#include "global.h"
#include "fieldservice.h"
#include <QMessageBox>


namespace Ui {
class FieldDialog;
}

class FieldDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FieldDialog(QString dbName="",QString tableName="",QString oldFieldName="",FieldEntity *fieldEntity = NULL,QWidget *parent = 0);
    ~FieldDialog();

signals:
    void sendField(FieldEntity* field); //向父窗口发送字段实体用于显示
private slots:
    void save();
    void cancel();

    //新增
    void reSetBasic(); //重置基础选项设置

    void checkPK(int state);
    void checkNull(int state);
    void checkSuoYin(int state);

private:
    Ui::FieldDialog *ui;
    void init();
    bool dealFieldType(int type);
    void solveSuoYin(QString path,FieldEntity* field);
    FieldEntity* fieldEntity;
    QString dbName;
    QString tableName;
    QString oldFieldName;
};

#endif // FIELDDIALOG_H
