#include "comparelinedialog.h"
#include "ui_comparelinedialog.h"
#include "data/datacenter.h"
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QDateTimeAxis>
#include <QtCharts/QValueAxis>
#include <iostream>

int COMPARE_MONTH_LENGTH = 2;

QT_CHARTS_USE_NAMESPACE

// 初始化图表
void initQChart(QChart* chart, std::vector<QString> codeList) {
    DataCenter& instance = DataCenter::getInstance();
    QSqlDatabase&& db = DataCenter::createDatabase();
    QDate currDate = QDate::currentDate();
    currDate = currDate.addMonths(-COMPARE_MONTH_LENGTH);
    std::string dateWhereInfo(" and trade_date>'");
    std::string dateStr = currDate.toString("yyyyMMdd").toStdString();
    dateWhereInfo.append(dateStr);

    QDateTimeAxis *axisX = new QDateTimeAxis;
    axisX->setTickCount(10);
    axisX->setFormat("MMM yyyy");
    axisX->setTitleText("Date");
    chart->addAxis(axisX, Qt::AlignBottom);

    QValueAxis *axisY = new QValueAxis;
    axisY->setLabelFormat("%i");
    axisY->setTitleText("Sunspots count");
    axisY->setTickCount(10);
    chart->addAxis(axisY, Qt::AlignLeft);

    double max_val = 0, min_val = 100000;

    auto begin_ite = codeList.begin(), end_ite = codeList.end();
    while(begin_ite != end_ite) {
        StockBatchInfo stockInfo = instance.getStockDayInfo((*begin_ite).toStdString(), db, dateWhereInfo);
        QLineSeries *series = new QLineSeries();

        // 设置数据
        for(int i = 0;i < stockInfo.getLength();i++) {
            StockBatchInfo::SingleInfo* info = stockInfo.getOneDayInfo(i);
            QDate& date = info->tradeDate;
            QDateTime dateTime(date);
            qint64 val = dateTime.toMSecsSinceEpoch();
            series->append(val, info->close);

            //判定一下最大最小值
            max_val = max_val < info->close ? info->close : max_val;
            min_val = min_val < info->close ? min_val : info->close;
        }

        series->setName(*begin_ite);
        chart->addSeries(series);

        series->attachAxis(axisX);

        series->attachAxis(axisY);

        begin_ite++;
    }
    axisY->setMax(max_val);
    axisY->setMin(min_val);
}

CompareLineDialog::CompareLineDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CompareLineDialog)
{
    ui->setupUi(this);
}

CompareLineDialog::~CompareLineDialog()
{
    delete ui;
}

void CompareLineDialog::accept() {
    QChart *chart = new QChart();

    QString firstStock = ui->lineEdit->text();
    QString secondStock = ui->lineEdit_2->text();
    QString thridStock = ui->lineEdit_3->text();
    std::vector<QString> content;
    content.push_back(firstStock);
    content.push_back(secondStock);
    content.push_back(thridStock);

    initQChart(chart, content);
    QChartView* view = new QChartView(chart);
    //view->setRubberBand(QChartView::HorizontalRubberBand);
    view->resize(900, 900);

    view->show();
}
