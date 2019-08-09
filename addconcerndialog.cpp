#include "addconcerndialog.h"
#include "ui_addconcerndialog.h"

AddConcernDialog::AddConcernDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddConcernDialog)
{
    ui->setupUi(this);
    // 配置监听函数
    //i->
}

AddConcernDialog::~AddConcernDialog()
{
    delete ui;
}
