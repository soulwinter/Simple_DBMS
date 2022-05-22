#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include "dbcreatediqlog.h"
#include "tbcreatedialog.h"
#include <QTreeWidgetItem>
#include <QTableWidgetItem>
#include <tbmodifynamedialog.h>
#include "recordcreatedialog.h"
#include "selectdialog.h"
#include "sqlanalyze.h"
#include "export.h"

namespace Ui {
class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QString username,QWidget *parent = 0);
    ~Dialog();

private slots:
    void addItem(); //新建数据库
    void addTable();//新建表
    void modifyTable();//修改表结构
    void showDatabase(QString databaseName);
    void test();
    void changeDatabaseEdit(QTreeWidgetItem* item,int column);
    void flush();//全局刷新
    void flushTable(QString dbName,QString tableName);//刷新表列表
    void addTableChildByDB(DBEntity dbEntity,QTreeWidgetItem* item); //根据数据库展示表
    void rightClickMenu(QTreeWidgetItem* item); //弹出菜单，不是右键，是双击，当时命名错了
    void doubleClickRecord(QTableWidgetItem* item); //选中记录
    void deleteItem();//删除数据库或者表
    void modifyItemName();//修改数据库或者表的名字
    void addRecord(); //创建字段
    void openTable(); //打开表，即显示所有记录
    void deleteRecord(); //删除记录
    void updateRecord(); //修改记录

    //新增
    void selectRecord(); //打开查询记录的窗口
    void flushSelectView(vector<RecordEntity> result); //查询后刷新进行显示
    void flushSelectViewSelective(vector<RecordEntity> result,vector<QString> needView); //查询后刷新进行显示
    QStringList getUserInfoByUsername(QString username);
    void outputDB();//导出数据库
    void inputDB();//导入数据库

    //sql解析
    void doSql();

private:
    QTreeWidgetItem* curItem; //用来记录当前被点击的treeItem
    QTableWidgetItem* curTableItem = NULL; //用来记录当前被点击的tableItem
    Ui::Dialog *ui;
    DBCreateDiqlog* dBcreateDialog;
    TBCreateDialog* tbcreateDialog;

    //用于删除记录
    QString deleteDbName = "";
    QString deleteTableName = "";
    void init();

    //记录当前用户
    QString username;
};

#endif // DIALOG_H
