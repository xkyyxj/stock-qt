#include <QSqlDatabase>

#include "data/datacenter.h"
#include "addconcerndialog.h"
#include "ui_addconcerndialog.h"

static std::string CONCERN_STOCK_TABLE_NAME = "concern_stock";

AddConcernDialog::AddConcernDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddConcernDialog)
{
    ui->setupUi(this);
}

AddConcernDialog::~AddConcernDialog()
{
    delete ui;
}

void AddConcernDialog::accept() {
    QSqlDatabase defaultDatabase;
    DataCenter& instance = DataCenter::getInstance();
    // 添加到自选的逻辑
    QString ts_code = ui->ts_code->text();
    QString reason = ui->add_reason->toPlainText();

    // 插入之前校验一下是否已经存在
    bool isExist = false;
    QString isExistCheck("select 1 from ");
    isExistCheck.append(QString::fromStdString(CONCERN_STOCK_TABLE_NAME));
    isExistCheck.append(" where ts_code='").append(ts_code).append("'");
    isExistCheck.append(" and remove_date is null");
    instance.executeQuery(isExistCheck, [&isExist](QSqlQuery& query) -> void {
        while(query.next()) {
            isExist = true;
        }
    }, defaultDatabase);

    // TODO -- 可以尝试去更新一下添加原因
    if(isExist) {
        QDialog::accept();
        return;
    }

    // emmmmm……查询一下名称是什么
    QString ts_name;
    QString sql("select name from stock_list where ts_code='");
    sql.append(ts_code).append("'");
    instance.executeQuery(sql, [&ts_name](QSqlQuery& query) -> void {
        while(query.next()) {
            ts_name = query.value("name").toString();
        }
    }, defaultDatabase);

    // 执行数据库插入逻辑
    std::vector<std::string> columns;
    columns.push_back("ts_code");
    columns.push_back("ts_name");
    columns.push_back("add_date");
    columns.push_back("reason");
    columns.push_back("is_remove");
    columns.push_back("remove_date");
    columns.push_back("remove_reason");

    std::vector<QVariant> values;
    values.push_back(ts_code);
    values.push_back(ts_name);
    values.push_back(QDate::currentDate());
    values.push_back(reason);
    values.push_back("N");
    values.push_back(QVariant());
    values.push_back(QVariant());

    instance.executeInsertOne(CONCERN_STOCK_TABLE_NAME, columns, values,
                              defaultDatabase);
    QDialog::accept();
}
