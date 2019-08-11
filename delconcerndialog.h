#ifndef DELCONCERNDIALOG_H
#define DELCONCERNDIALOG_H

#include <QDialog>

namespace Ui {
class DelConcernDialog;
}

class DelConcernDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DelConcernDialog(QWidget *parent = nullptr);
    ~DelConcernDialog();
public slots:
    void accept();

private:
    Ui::DelConcernDialog *ui;
};

#endif // DELCONCERNDIALOG_H
