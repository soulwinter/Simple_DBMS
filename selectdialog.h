#ifndef SELECTDIALOG_H
#define SELECTDIALOG_H

#include <QDialog>
#include <vector>
#include <map>
#include "recordservice.h"
#include "recordentity.h"
#include "fieldservice.h"

namespace Ui {
class SelectDialog;
}

class SelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectDialog(QString dbName,QString tableName,QWidget *parent = 0);
    ~SelectDialog();

private:
    QString dbName;
    QString tableName;
    Ui::SelectDialog *ui;
    void init(); //初始化查询界面
signals:
    void sendSelectInfo(vector<RecordEntity>);    //返回信息给主界面进行显示
private slots:
    void cancel();
    void select();

};

#endif // SELECTDIALOG_H
