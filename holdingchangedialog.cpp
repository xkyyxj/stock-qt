#include "holdingchangedialog.h"
#include "ui_holdingchangedialog.h"

HoldingChangeDialog::HoldingChangeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::HoldingChangeDialog)
{
    ui->setupUi(this);
}

HoldingChangeDialog::~HoldingChangeDialog()
{
    delete ui;
}

void HoldingChangeDialog::accept() {
    QString ts_code = ui->ts_code->text();
    //bool isReal = ui->is_real->
}
