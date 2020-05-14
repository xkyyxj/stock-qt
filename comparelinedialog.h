#ifndef COMPARELINEDIALOG_H
#define COMPARELINEDIALOG_H

#include <QDialog>

namespace Ui {
class CompareLineDialog;
}

class CompareLineDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CompareLineDialog(QWidget *parent = nullptr);
    ~CompareLineDialog();

public slots:
    void accept();

private:
    Ui::CompareLineDialog *ui;
};

#endif // COMPARELINEDIALOG_H
