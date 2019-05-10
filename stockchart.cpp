#include "stockchart.h"

static void paintSingleKLine(QPainter* painter, int x, int y, int width, int height, StockBatchInfo::SingleInfo& info) {

}

StockChart::StockChart(QWidget* parent) : QOpenGLWidget(parent) {
    model = nullptr;
}

StockChart::~StockChart() {
    if(model) {
        delete model;
    }
}

void StockChart::setModel(StockChartModel *_model) {
    model = _model;
    connect(model, SIGNAL(dataChanged), this, SLOT(stockInfoChanged));
}


void StockChart::stockInfoChanged() {
    this->repaint();
}

void StockChart::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 开始绘制之旅
    // 首先就是K线图
    if(currDisplayType == DisplayType::K_TYPE) {
        paintKLine(&painter, event);
    }
    // 其次就是分时图
    else if(currDisplayType == DisplayType::INDEX_TYPE) {
        paintIndexLine(&painter, event);
    }
    painter.end();
}

void StockChart::paintKLine(QPainter* painter, QPaintEvent *event) {
    StockBatchInfo* kInfo = model->getCurrStockKInfo();

    // 总长宽
    int startx = event->rect().x();
    int starty = event->rect().y();
    int totalWidth = event->rect().width();
    int totalHeight =event->rect().height();

    int eachWidth = totalWidth / kInfo->info_list.size();

    // 统计一下最大和最小价差
    float maxPrice = 100000, minPrice = 0;
    for(int i = 0;i < kInfo->info_list.size();i++) {
        maxPrice = kInfo->info_list[i].high > maxPrice ? kInfo->info_list[i].high : maxPrice;
        minPrice = kInfo->info_list[i].low > minPrice ? minPrice : kInfo->info_list[i].high;
    }
    float minMaxDelta = maxPrice - minPrice;

    // 每根K线的窗格Rect
    int x, y, height;
    for(int i = 0;i < kInfo->info_list.size();i++) {
        x = startx + eachWidth * i;
        // 计算一下单根K线的Y值和height属性
        y = starty + static_cast<int>((maxPrice - kInfo->info_list[i].high) / minMaxDelta * totalHeight);
        height = static_cast<int>((kInfo->info_list[i].high - kInfo->info_list[i].low) / minMaxDelta * totalHeight);
        paintSingleKLine(painter, x, y, eachWidth, height, kInfo->info_list[i]);
    }
    event->rect().x();
}
void StockChart::paintIndexLine(QPainter* painter, QPaintEvent *event) {
    StockIndexBatchInfo* indexInfo = model->getCurrStockIndexInfo();

    // 统计一下最大和最小价差
    float maxPrice = 100000, minPrice = 0;
    for(int i = 0;i < indexInfo->info_list.size();i++) {
        maxPrice = indexInfo->info_list[i].price > maxPrice ? indexInfo->info_list[i].price : maxPrice;
        minPrice = indexInfo->info_list[i].price > minPrice ? minPrice : indexInfo->info_list[i].price;
    }

    for(int i = 0;i < indexInfo->info_list.size();i++) {

    }
}

