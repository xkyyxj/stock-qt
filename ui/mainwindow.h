#pragma execution_character_set("utf-8")
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlRelationalTableModel>
#include "ui/maintablemodel.h"
#include "stockchartmodel.h"
#include <QModelIndex>
#include <QItemSelection>
#include "data/stockinfo.h"
#include "infodisplay.h"
#include "utils/rediscachetools.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

    RedisCacheTools cacheTools;
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void anaRstTypeSelect(const std::string&, const std::string&) noexcept;
    void initTableModel();
    void initMenuAction();

private slots:
    void tableDoubleClicked(const QModelIndex& index);
    void setFetchIndexDelta();
    void stockInfoSelected(StockInfo&);

    void selectedStockChanged();
public slots:
    void treeNodeSelected(const QItemSelection &selected, const QItemSelection &deselected);

    void startCalculate();
    void showCompareCharts();

private:
    Ui::MainWindow *ui;
    InfoDisplay* display;
    //QSqlRelationalTableModel* tableModel;
    MainTableModel* tableModel;
    StockChartModel* viewModel;
};

#endif // MAINWINDOW_H
