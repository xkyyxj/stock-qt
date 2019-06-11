#include "stockchart.h"
#include <iostream>

const int StockChart::MIN_K_LINE_WIDTH = 5;
const int StockChart::DEFAULT_K_LINE_WIDTH = 20;
const int StockChart::MAX_K_LINE_WIDTH = 25;

/*
 * 鉴于如果要显示的K线太多的话，那么可能会显示不过来，那么判定一下，最多显示多少条
 */
void StockChart::judgeDisplay(const QRect& rect, StockBatchInfo* kInfo) {
    int showNum = endIndex - startIndex;
    showNum = showNum <= 0 ? kInfo->info_list.size() : showNum;
    int eachWidth = rect.width() / showNum;
    if(eachWidth < MIN_K_LINE_WIDTH) {
        eachWidth = DEFAULT_K_LINE_WIDTH;
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
    else {
        // 此处填充一下背景色，以防分割窗口的横线影响K线的显示
        QRect fillRect(x + padding + 1, main_start_y + 1, width - 2 * padding - 1, mainHeight - 1);
        painter->fillRect(fillRect, Qt::white);
    }


    // 计算并绘制下影线部分
    int down_start_y = main_start_y + mainHeight, down_end_y = y + height;
    QLine downLine(up_start_x, down_start_y, up_start_x, down_end_y);
    painter->drawLine(downLine);
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

    startIndex = endIndex = -1;
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

    // 绘制一下窗格系统，将窗格横向分割成10个小窗格
    QPen pen;
    QColor color;
    color.setRgb(212, 208, 208);
    pen.setColor(color);
    painter->setPen(pen);

    int eachWinHeight = totalHeight / 10;
    for(int i = 1;i <= 10;i++) {
        QLine tempLine(0, eachWinHeight * i, event->rect().width(), eachWinHeight * i);
        painter->drawLine(tempLine);
    }


    // 每根K线的窗格Rect
    int x, y, height;
    for(int i = startIndex;i <= endIndex;i++) {
        x = startx + eachLineWidth * (i - startIndex);
        // 计算一下单根K线的Y值和height属性
        y = starty + static_cast<int>((maxPrice - kInfo->info_list[i].high) / minMaxDelta * totalHeight);
        height = static_cast<int>((kInfo->info_list[i].high - kInfo->info_list[i].low) / minMaxDelta * totalHeight);
        paintSingleKLine(painter, x, y, eachLineWidth, height, kInfo->info_list[i]);
    }

    // 绘制一下当前鼠标所在K线的位置，加一个十字线，贯穿整个窗口 TODO-- 还不行
    std::cout << currMouseP.x() << " " << currMouseP.y() << std::endl;
    int kLineNum = currMouseP.x() / eachLineWidth;
    int pointX = kLineNum * eachLineWidth * eachLineWidth / 2;
    QLine horizonLine(0, currMouseP.y(), event->rect().width(), currMouseP.y());
    painter->drawLine(horizonLine);

    std::cout << "pointX is " << pointX << std::endl;
    QLine verticLine(pointX, 0, pointX, event->rect().height());
    painter->drawLine(verticLine);
}

void StockChart::paintIndexLine(QPainter* painter, QPaintEvent *event) {
    StockIndexBatchInfo* indexInfo = model->getCurrStockIndexInfo();

    // 沪深两市总交易时长（单位：秒）
    int totalSeconds = 4 * 60 * 60;

    // 开市时间
    QString today = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    const QDateTime moringStartTime = QDateTime::fromString(today + " 09:30:00");
    const QDateTime afternooneStartTime = QDateTime::fromString(today + " 13:00:00");

    // 统计一下最大和最小价差
    float maxPrice = 0, minPrice = 100000, delta = 0;
    for(int i = 0;i < indexInfo->infoList.size();i++) {
        maxPrice = indexInfo->infoList[i].price > maxPrice ? indexInfo->infoList[i].price : maxPrice;
        minPrice = indexInfo->infoList[i].price > minPrice ? minPrice : indexInfo->infoList[i].price;
    }
    delta = maxPrice - minPrice;

    int preX = 0, preY = 0; // 上一根线条的结束点
    for(int i = 1;i < indexInfo->infoList.size();i++) {
        int y = static_cast<int>((maxPrice - indexInfo->infoList[i].price) / delta * event->rect().height());

        // 计算一下经历了多长的交易时间(午休时间不计算在内)
        int deltaTime = 0;
        // 上午交易时间段内
        if(indexInfo->infoList[i].time < afternooneStartTime) {
            deltaTime += indexInfo->infoList[i].time.secsTo(moringStartTime);
        }
        // 下午交易时间段内
        else {
            deltaTime += totalSeconds / 2 +  indexInfo->infoList[i].time.secsTo(afternooneStartTime);
        }

        int x = deltaTime / totalSeconds * event->rect().width();

        QLine tempLine(preX, preY, x, y);
        painter->drawLine(tempLine);
        preX = x;
        preY = y;
    }
}

void StockChart::mouseMoveEvent(QMouseEvent *event) {
    std::cout << "mouseMoveEvent: " << event->pos().x() << event->pos().y() << std::endl;
    currMouseP = event->pos();
    update();
}

void StockChart::keyReleaseEvent(QKeyEvent *event) {
    StockBatchInfo* kInfo = model->getCurrStockKInfo();
    // 每次按下Up或者Down键，变动十根K线
    int totalChangeItemCount = 10;
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
            // 当K线的宽度小于最大K线宽度的时候才能够放大
            if(eachLineWidth < MAX_K_LINE_WIDTH) {
                if(endIndex - startIndex > 20) {
                    startIndex += 5;
                    endIndex  -= 5;
                }
                update();
            }
            break;
        case Qt::Key_Down:
            // 当K线的宽度大于最小K线宽度的时候才能够缩小
            if(eachLineWidth > MIN_K_LINE_WIDTH) {
                if(startIndex > 5) {
                    startIndex -= 5;
                    totalChangeItemCount -= 5;
                }
                else {
                    totalChangeItemCount -= startIndex;
                    startIndex = 0;
                }
                endIndex = (endIndex + totalChangeItemCount) < kInfo->info_list.size() ? endIndex + totalChangeItemCount
                                                                                       : kInfo->info_list.size() - 1;
                update();
            }
            break;
        }
    }
}


void StockChart::resizeEvent(QResizeEvent */*event*/) {
    isFirstRender = true;
    update();
}

