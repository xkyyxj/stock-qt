#ifndef DATALINEDIALOG_H
#define DATALINEDIALOG_H

#include <QDialog>

namespace Ui {
class DataLineDialog;
}

class DataLineDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DataLineDialog(QWidget *parent = nullptr);
    ~DataLineDialog();

private:
    Ui::DataLineDialog *ui;
};

#endif // DATALINEDIALOG_H
