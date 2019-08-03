#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "categorytreemodel.h"
#include <QSqlDatabase>
#include <iostream>
#include <QVector>
#include <QItemSelectionModel>
#include <QInputDialog>
#include "calculator.h"
#include "utils/zlibcompress.h"

void MainWindow::startCalculate() {
    Calculator::startCalcualte();
}

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
    std::vector<QString> selectColumns;
    selectColumns.push_back("ana_category_detail.ts_code");
    tableModel = new MainTableModel(&dataCenter, this);
    tableModel->setTableName("ana_category_detail join stock_list on stock_list.ts_code=ana_category_detail.ts_code");
    tableModel->setSelectColumns(selectColumns);
    std::vector<QString> displayHead;
    displayHead.push_back("编码");
    tableModel->setDisplayHeadInfo(displayHead);
}

void MainWindow::initMenuAction() {
    // 计算按钮
    QAction* calculateAction = ui->actioncalculate;
    connect(calculateAction, &QAction::triggered, this, &MainWindow::startCalculate);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow), tableModel()
{
    // 测试一下ｚｌｉｂ关于压缩的错误(测试正确后，发现花费了十分钟执行下面的循环)
    /*QDateTime currTime = QDateTime::currentDateTime();
    std::cout << "begin" << std::endl;
    std::cout << currTime.toString("yyyy-MM-dd HH:mm:ss").toStdString() << std::endl;
    zlib::ZLibCompress compress;
    StockIndexBatchInfo batchInfo;
    for(int i = 0;i < 10000;i++) {
        StockIndexBatchInfo::SingleIndexInfo singleInfo;
        batchInfo.info_list.push_back(singleInfo);
        std::string strInfo = batchInfo.encodeToStr();
        compress.startCompress();
        std::vector<unsigned char> rst = compress.endCompress(strInfo);

        // 反解压一下，看下是不是ＯＫ的
        compress.startDecompress();
        std::vector<unsigned char> deRst = compress.endDecompress(rst.data(),rst.size());
        deRst.push_back('\0');
        unsigned char* realData = deRst.data();
        char* real_data = reinterpret_cast<char*>(realData);
        std::string val(real_data);
        if(!(val.at(0) == '%')) {
            std::cout << "decompress error!!!:" << i << std::endl;
        }
    }

    QDateTime currTime2 = QDateTime::currentDateTime();
    std::cout << "end" << std::endl;
    std::cout << currTime2.toString("yyyy-MM-dd HH:mm:ss").toStdString() << std::endl;*/

    ui->setupUi(this);

    // 初始化按钮控制
    initMenuAction();

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
    dataCenter.startFetchIndexInfo();

    // 开启实时分析程序
    //dataCenter.startExecIndexAna();

    // 控制下Ｋ线图的切换
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), ui->openGLWidget, SLOT(infoTypeChanged(int)));

    //　控制下鼠标移动到某根Ｋ线的时候，显示当日的信息
    connect(ui->openGLWidget, SIGNAL(mouseOnChanged(StockInfo&)),
            this, SLOT(stockInfoSelected(StockInfo&)));

    // 创建一个弹出窗，然后显示单根Ｋ线的信息
    display = new InfoDisplay(this);
    display->setModal(Qt::NonModal);
    display->show();

    connect(ui->openGLWidget, SIGNAL(mouseOnChanged(StockInfo&)),
            display, SLOT(stockInfoChanged(StockInfo&)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::selectedStockChanged() {
    QString ts_code = ui->lineEdit->text();
    viewModel->setSelectedStock(ts_code.toStdString());
}

void MainWindow::tableDoubleClicked(const QModelIndex& index) {
    viewModel->currSelectdStockChanged(index);
}

void MainWindow::stockInfoSelected(StockInfo& info) {
    // 构建一个展示字符串
    QString content;
    content.append("名称:").append(info.ts_name).append(";");
    ui->label->setText(content);
}

void MainWindow::treeNodeSelected(const QItemSelection &selected, const QItemSelection &deselected) {
    QModelIndex index = selected.indexes().first();
    QVariant varData = index.data(Category::IDRole);
    QString pkField = varData.toString();

    // 查询一下是哪张表
    QString tableName, pk_tablemeta;
    DataCenter& instance = DataCenter::getInstance();
    QString queryTableName("select table_name, ana_category.pk_tablemeta from ana_category join table_meta on ana_category.pk_tablemeta="
                  "table_meta.pk_tablemeta where pk_category='");
    queryTableName.append(pkField).append("'");
    instance.executeQuery(queryTableName, [&tableName, &pk_tablemeta](QSqlQuery& query) -> void {
        while(query.next()) {
            tableName = query.value("table_name").toString();
            pk_tablemeta = query.value("pk_tablemeta").toString();
        }
    }, QSqlDatabase());

    std::vector<QString> selectedColumns, tableHead;
    QString tableDetailInfoQry("select * from table_column where pk_tablemeta='");
    tableDetailInfoQry.append(pk_tablemeta).append("'");
    instance.executeQuery(tableDetailInfoQry, [&selectedColumns, &tableHead](QSqlQuery& query) -> void {
        while(query.next()) {
            selectedColumns.push_back(query.value("column_name").toString());
            tableHead.push_back(query.value("display_name").toString());
        }
    }, QSqlDatabase());

    QDate currDate = QDate::currentDate();
    QString dateStr = currDate.toString("yyyy-MM-dd");
    QString filter(" where date='");
    filter.append(dateStr).append("'");

    // 通知表格model进行数据更新
    if(tableName.size() > 0) {
        tableModel->setTableName(tableName);
        tableModel->setSelectColumns(selectedColumns);
        tableModel->setDisplayHeadInfo(tableHead);
        tableModel->setFilter(filter);
        tableModel->selectData();
    }
}

void MainWindow::setFetchIndexDelta() {
    bool ok = false;
    QString string = QInputDialog::getText(this,"4535345","24234",QLineEdit::Normal,"45345",&ok);
}
