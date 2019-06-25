#pragma execution_character_set("utf-8")
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlRelationalTableModel>
#include "ui/maintablemodel.h"
#include "stockchartmodel.h"
#include <QModelIndex>
#include <QItemSelection>

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
    void initMenuAction();

private slots:
    void tableDoubleClicked(const QModelIndex& index);
    void setFetchIndexDelta();
public slots:
    void treeNodeSelected(const QItemSelection &selected, const QItemSelection &deselected);

private:
    Ui::MainWindow *ui;
    //QSqlRelationalTableModel* tableModel;
    MainTableModel* tableModel;
    StockChartModel* viewModel;
};

#endif // MAINWINDOW_H
