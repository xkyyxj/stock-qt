#ifndef ADDCONCERNDIALOG_H
#define ADDCONCERNDIALOG_H

#include <QDialog>

namespace Ui {
class AddConcernDialog;
}

class AddConcernDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddConcernDialog(QWidget *parent = nullptr);
    ~AddConcernDialog();

    void setTsCode(QString ts_code) noexcept;

public slots:
    void accept();

private:
    Ui::AddConcernDialog *ui;
};

#endif // ADDCONCERNDIALOG_H
