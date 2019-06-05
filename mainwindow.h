#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlRelationalTableModel>
#include <maintablemodel.h>
#include "stockchartmodel.h"
#include <QModelIndex>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void initTableModel();

private slots:
    void tableDoubleClicked(const QModelIndex& index);

private:
    Ui::MainWindow *ui;
    //QSqlRelationalTableModel* tableModel;
    MainTableModel* tableModel;
    StockChartModel* viewModel;
};

#endif // MAINWINDOW_H
