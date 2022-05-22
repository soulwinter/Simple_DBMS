#ifndef RECORDCREATEDIALOG_H
#define RECORDCREATEDIALOG_H

#include <QDialog>
#include "recordservice.h"
#include "recordentity.h"
#include "fieldservice.h"
#include "fieldentity.h"


namespace Ui {
class RecordCreateDialog;
}

class RecordCreateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RecordCreateDialog(RecordEntity recordEntity,QString databaseName = "",QString tableName = "",QWidget *parent = 0);
    ~RecordCreateDialog();

private:
    Ui::RecordCreateDialog *ui;
    QString databaseName;
    QString tableName;
    RecordEntity recordEntity; //用来判断是新增还是修改
    void init();
private slots:
    void save();
    void cancel();
};

#endif // RECORDCREATEDIALOG_H
