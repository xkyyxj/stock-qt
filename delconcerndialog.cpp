#include "delconcerndialog.h"
#include "ui_delconcerndialog.h"

#include <QSqlDatabase>
#include "data/datacenter.h"

static std::string CONCERN_STOCK_TABLE_NAME = "concern_stock";

DelConcernDialog::DelConcernDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DelConcernDialog)
{
    ui->setupUi(this);
}

DelConcernDialog::~DelConcernDialog()
{
    delete ui;
}

void DelConcernDialog::accept() {
    QSqlDatabase defaultDatabase;
    DataCenter& instance = DataCenter::getInstance();
    // 添加到自选的逻辑
    QString ts_code = ui->ts_code->text();
    QString reason = ui->del_reason->toPlainText();

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

    if(!isExist) {
        QDialog::accept();
        return;
    }

    // 执行移除逻辑
    std::vector<std::string> columns;
    columns.push_back("is_remove");
    columns.push_back("remove_date");
    columns.push_back("remove_reason");

    std::vector<QVariant> values;
    values.push_back("Y");
    values.push_back(QDate::currentDate());
    values.push_back(reason);

    std::string wherePart("where ts_code='");
    wherePart.append(ts_code.toStdString()).append("'");
    instance.executeUpdate(CONCERN_STOCK_TABLE_NAME, columns, values,
                           wherePart, defaultDatabase);

    QDialog::accept();
}
