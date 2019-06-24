#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "categorytreemodel.h"
#include <QSqlDatabase>
#include <iostream>
#include <QVector>
#include <QItemSelectionModel>
#include <QInputDialog>

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
    DataCenter& dataCenter = DataCenter::getInstance();
    QVector<QString> selectColumns;
    selectColumns.push_back("ana_category_detail.ts_code");
    tableModel = new MainTableModel(&dataCenter, this);
    tableModel->setTableName("ana_category_detail join stock_list on stock_list.ts_code=ana_category_detail.ts_code");
    tableModel->setSelectColumns(selectColumns);
    QVector<QString> displayHead;
    displayHead.push_front("编码");
    tableModel->setDisplayHeadInfo(displayHead);
}

void MainWindow::initMenuAction() {

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
    DataCenter& dataCenter = DataCenter::getInstance();
    viewModel = new StockChartModel();
    ui->openGLWidget->setModel(viewModel);

    // 初始化相关信号
    // 表格双击某行的时候，图表当中展示改行股票的K线图或者折线图信息
    connect(ui->tableView, SIGNAL(doubleClicked(const QModelIndex &)),viewModel, SLOT(currSelectdStockChanged(const QModelIndex&)));
    connect(ui->treeView->selectionModel(),SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
            this,SLOT(treeNodeSelected(const QItemSelection &, const QItemSelection &)));

    // 开始获取股票的实时信息
    //dataCenter.startFetchIndexInfo();

    // 尝试网Redis当中写入数据
    std::string temp_val("6666666");
    RedisCacheTools tools;
    bool isOK = tools.writeStrToRedis("123", temp_val);
    if(isOK) {
        tools.getBinaryDataFromRedis("123", [](char* val) -> void {
            std::string tempStr(val);
            std::cout << tempStr << std::endl;
        });
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::tableDoubleClicked(const QModelIndex& index) {
    viewModel->currSelectdStockChanged(index);
}


void MainWindow::treeNodeSelected(const QItemSelection &selected, const QItemSelection &deselected) {
    QModelIndex index = selected.indexes().first();
    QVariant varData = index.data(Category::IDRole);
    QString pkField = varData.toString();

    // 通知表格model进行数据更新
    tableModel->setFilter("where pk_category='" + pkField + "'");
    tableModel->selectData();
}

void MainWindow::setFetchIndexDelta() {
    bool ok = false;
    QString string = QInputDialog::getText(this,"4535345","24234",QLineEdit::Normal,"45345",&ok);
}
