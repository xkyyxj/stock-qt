#include "stockchart.h"
#include <iostream>

const int StockChart::MIN_K_LINE_WIDTH = 20;
const int StockChart::DEFAULT_K_LINE_WIDTH = 20;

/*
 * 鉴于如果要显示的K线太多的话，那么可能会显示不过来，那么判定一下，最多显示多少条
 */
void StockChart::judgeDisplay(const QRect& rect, StockBatchInfo* kInfo) {
    int eachWidth = rect.width() / kInfo->info_list.size();
    if(eachWidth <= MIN_K_LINE_WIDTH) {
        eachWidth = MIN_K_LINE_WIDTH;
    }

    eachLineWidth = eachWidth;
}

static void paintSingleKLine(QPainter* painter, int x, int y, int width, int height, StockBatchInfo::SingleInfo& info) {
    float delta = info.high - info.low;

    // 判定用什么颜色的画笔
    QPen pen;
    QColor color;
    if(info.close > info.open) {
        color.setRgb(217, 58, 24);
    }
    else {
        color.setRgb(7, 122, 50);
    }
    pen.setColor(color);
    painter->setPen(pen);

    // 计算并绘制上影线部分
    float up_delta = info.high - (info.open > info.close ? info.open : info.close);
    float up_pct = up_delta / delta;
    int up_start_x, up_start_y, up_end_y;
    up_start_x = x + width / 2;
    up_start_y = y;
    up_end_y = y + static_cast<int>(height * up_pct);
    QLine upLine(up_start_x, up_start_y, up_start_x, up_end_y);
    painter->drawLine(upLine);

    //计算并绘制实体部分
    float main_delta = info.open > info.close ? info.open - info.close : info.close - info.open;
    float main_pct = main_delta / delta;
    int main_start_y = up_end_y, mainHeight = static_cast<int>(main_pct * height);
    int padding = static_cast<int>(width * 0.1);
    QRect mainRect(x + padding, main_start_y, width - 2 * padding, mainHeight);
    painter->drawRect(mainRect);
    if(info.close < info.open) {
        painter->fillRect(mainRect, color);
    }


    // 计算并绘制下影线部分
    int down_start_y = main_start_y + mainHeight, down_end_y = y + height;
    QLine downLine(up_start_x, down_start_y, up_start_x, down_end_y);
    painter->drawLine(downLine);
}

static void drawPriceLine(QPainter* painter, const QRect& paintArea, float minPrice = 0, float maxPrice=50) {
    int height = paintArea.height() / 10;
    for(int i = 0;i < 10;i++) {
        QLine tempLine(0, paintArea.y(), paintArea.x() + paintArea.width(), paintArea.y() + i * height);
        painter->drawLine(tempLine);
    }
}

StockChart::StockChart(QWidget* parent) : QFrame(parent) {
    model = nullptr;
    QPalette pal;
    pal.setColor(QPalette::Background, Qt::white);
    setAutoFillBackground(true);
    setPalette(pal);

    setFocusPolicy(Qt::ClickFocus);

    currDisplayType = DisplayType::K_TYPE;
    isFirstRender = true;
}

StockChart::~StockChart() {
    if(model) {
        delete model;
    }
}

void StockChart::setModel(StockChartModel *_model) {
    model = _model;
    connect(model, SIGNAL(dataChanged()), this, SLOT(stockInfoChanged()));
}


void StockChart::stockInfoChanged() {
    isFirstRender = true;
    this->repaint();
}

void StockChart::paintEvent(QPaintEvent *event)
{
    QPainter painter;
    painter.begin(this);
    //painter.setRenderHint(QPainter::Antialiasing);

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
    int startx = 0;
    int starty = 0;
    int totalHeight =event->rect().height();

    judgeDisplay(event->rect(), kInfo);
    int displayNum = event->rect().width() / eachLineWidth;
    if(isFirstRender) {
        //统计一下显示区间
        endIndex = kInfo->info_list.size() - 1;
        startIndex = endIndex - displayNum + 1;
        isFirstRender = false;
    }

    // 统计一下最大和最小价差
    float maxPrice = 0, minPrice = 1000000;
    for(int i = startIndex;i <= endIndex;i++) {
        maxPrice = kInfo->info_list[i].high > maxPrice ? kInfo->info_list[i].high : maxPrice;
        minPrice = kInfo->info_list[i].low > minPrice ? minPrice : kInfo->info_list[i].low;
    }
    float minMaxDelta = maxPrice - minPrice;

    // 每根K线的窗格Rect
    int x, y, height;
    for(int i = startIndex;i <= endIndex;i++) {
        x = startx + eachLineWidth * (i - startIndex);
        // 计算一下单根K线的Y值和height属性
        y = starty + static_cast<int>((maxPrice - kInfo->info_list[i].high) / minMaxDelta * totalHeight);
        height = static_cast<int>((kInfo->info_list[i].high - kInfo->info_list[i].low) / minMaxDelta * totalHeight);
        paintSingleKLine(painter, x, y, eachLineWidth, height, kInfo->info_list[i]);
    }
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

void StockChart::mouseMoveEvent(QMouseEvent *event) {
    int x = event->pos().x();
    int y = event->y();
}

void StockChart::keyReleaseEvent(QKeyEvent *event) {
    StockBatchInfo* kInfo = model->getCurrStockKInfo();
    if(currDisplayType == DisplayType::K_TYPE) {
        switch(event->key()) {
        case Qt::Key_Left:
            if(startIndex > 0) {
                startIndex -= 1;
                endIndex -= 1;
                update();
            }
            break;
        case Qt::Key_Right:
            if(endIndex < kInfo->info_list.size() - 1) {
                startIndex += 1;
                endIndex += 1;
                update();
            }
            break;
        case Qt::Key_Up:

            break;
        case Qt::Key_Down:
            break;
        }
    }
}

