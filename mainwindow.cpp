#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "categorytreemodel.h"
#include <QSqlDatabase>
#include <iostream>
#include <QVector>

extern DataCenter dataCenter;

void MainWindow::initTableModel() {
    /*QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "db");
    db.setHostName("localhost");
    db.setDatabaseName("stock");
    db.setUserName("root");
    db.setPassword("123");
    std::cout << "22222222" << std::endl;
    if(!db.open()) {
        std::cout << "123123123" << std::endl;
    }
    tableModel = new QSqlRelationalTableModel(this, db);
    tableModel->setTable("category_content");
    tableModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    //tableModel->setRelation(2, QSqlRelation("stock_list", "ts_code", "ts_code"));
    tableModel->setHeaderData(0, Qt::Horizontal, tr("pk"));
    tableModel->setHeaderData(1, Qt::Horizontal, tr("Salary"));
    tableModel->setHeaderData(2, Qt::Horizontal, tr("pk11"));
    tableModel->select();*/
    QVector<QString> selectColumns;
    selectColumns.push_back("category_content.ts_code");
    tableModel = new MainTableModel(&dataCenter, this);
    tableModel->setTableName("category_content join stock_list on stock_list.ts_code=category_content.ts_code");
    tableModel->setSelectColumns(selectColumns);
    QVector<QString> displayHead;
    displayHead.push_front("编码");
    tableModel->setDisplayHeadInfo(displayHead);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), tableModel()
{
    ui->setupUi(this);

    //构建树结构
    CategoryTreeModel* categoryModel = new CategoryTreeModel();
    ui->treeView->setModel(categoryModel);

    // 构建table的model
    initTableModel();
    ui->tableView->setModel(tableModel);
    tableModel->selectData();

    // 构建图形显示的model
    viewModel = new StockChartModel(&dataCenter);
    ui->openGLWidget->setModel(viewModel);

    // 初始化相关信号
    connect(ui->tableView, SIGNAL(doubleClicked(const QModelIndex &)),viewModel, SLOT(currSelectdStockChanged(const QModelIndex&)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::tableDoubleClicked(const QModelIndex& index) {
    viewModel->currSelectdStockChanged(index);
}
