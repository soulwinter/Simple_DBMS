#ifndef DBCREATEDIQLOG_H
#define DBCREATEDIQLOG_H

#include <QDialog>
#include <dbservice.h>
namespace Ui {
class DBCreateDiqlog;
}

class DBCreateDiqlog : public QDialog
{
    Q_OBJECT

public:
    explicit DBCreateDiqlog(QWidget *parent = 0);
    ~DBCreateDiqlog();

private:
    Ui::DBCreateDiqlog *ui;
    DBService* dbService;

signals:
    void sendData(QString databaseName);

private slots:
    void cancle();
    void create();
};

#endif // DBCREATEDIQLOG_H
