#ifndef TBMODIFYNAMEDIALOG_H
#define TBMODIFYNAMEDIALOG_H

#include <QDialog>
#include "dbservice.h"
#include "tableservice.h"
#include <QMessageBox>

namespace Ui {
class TBModifyNameDialog;
}

class TBModifyNameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TBModifyNameDialog(QString dbName="",QString oldName="",int type =1,QWidget *parent = 0);
    ~TBModifyNameDialog();

private:
    Ui::TBModifyNameDialog *ui;
    int type; //表示是修改表名还是修改数据库名
    QString oldName;
    QString dbName;
signals:
    void sendFlag();
private slots:
    void cancle();
    void save();
};

#endif // TBMODIFYNAMEDIALOG_H
