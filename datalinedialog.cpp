#include "datalinedialog.h"
#include "ui_datalinedialog.h"

DataLineDialog::DataLineDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DataLineDialog)
{
    ui->setupUi(this);
}

DataLineDialog::~DataLineDialog()
{
    delete ui;
}
