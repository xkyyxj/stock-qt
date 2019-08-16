#ifndef HOLDINGCHANGEDIALOG_H
#define HOLDINGCHANGEDIALOG_H

#include <QDialog>

namespace Ui {
class HoldingChangeDialog;
}

class HoldingChangeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit HoldingChangeDialog(QWidget *parent = nullptr);
    ~HoldingChangeDialog();

protected slots:
    void accept();

private:
    Ui::HoldingChangeDialog *ui;
};

#endif // HOLDINGCHANGEDIALOG_H
