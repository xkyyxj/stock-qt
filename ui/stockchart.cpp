#define BOOST_ALL_DYN_LINK
#include "stockchart.h"
#include <iostream>
#include <cmath>
#include <boost/chrono.hpp>

static const int MIN_K_LINE_WIDTH = 6;
static const int DEFAULT_K_LINE_WIDTH = 20;
static const int MAX_K_LINE_WIDTH = 25;

static const int INDEX_TWO_POINT_TIME_DELTA = 45;

// 价格字体的大小，12个像素
static const int FONT_SIZE = 12;

// 价格区域的宽度
static const int PRICE_AREA_WIDTH = 60;
// 价格区域对于左侧的边距
static const int PRICE_MARGIN_LEFT = 5;
// K线之间的间隔（亦即K线之间的空隙，固定为2像素（实则为4像素））
static const int KLINE_PADDING = 2;

/*
 * 鉴于如果要显示的K线太多的话，那么可能会显示不过来，那么判定一下，最多显示多少条
 */
void StockChart::judgeDisplay(const QRect& rect, StockBatchInfo* kInfo) {
    int showNum = endIndex - startIndex;
    showNum = showNum <= 0 ? kInfo->info_list.size() : ++showNum;
    int eachWidth = rect.width() / showNum;
    if(eachWidth < MIN_K_LINE_WIDTH) {
        eachWidth = DEFAULT_K_LINE_WIDTH;
    }
    eachLineWidth = eachWidth;

    // 对于剩余的空间，重新分配一下，以免右边会有很大的空白
    int currWidth = eachLineWidth * showNum;
    int leftWidth = rect.width() - currWidth;
    lineNumPartOne = leftWidth > 0 ? showNum / leftWidth : 0;
}

static void paintSingleKLine(QPainter* painter, int x, int y, int width, int height, StockBatchInfo::SingleInfo& info) {
    // 特殊处理一下涨停的效果，防止delta为０
    float delta = info.high - info.low > 0 ? info.high - info.low : -1;

    // 判定用什么颜色的画笔
    QPen pen;
    QColor color;
    if(info.close > info.open || (info.close >= info.open && info.pct_chg > 0)) {
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
    mainHeight = mainHeight == 0 ? 1 : mainHeight; //　确保主体部分有内容
    QRect mainRect(x + KLINE_PADDING, main_start_y, width - 2 * KLINE_PADDING, mainHeight);
    painter->drawRect(mainRect);
    if(info.close < info.open) {
        painter->fillRect(mainRect, color);
    }
    else {
        // 此处填充一下背景色，以防分割窗口的横线影响K线的显示
        QRect fillRect(x + KLINE_PADDING + 1, main_start_y + 1, width - 2 * KLINE_PADDING - 1, mainHeight - 1);
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

    // 下面这行代码使得鼠标不按下的时候也能跟踪鼠标移动事件
    setMouseTracking(true);
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

void StockChart::infoTypeChanged(int type) {
    currDisplayType = static_cast<DisplayType>(type);
    update();
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
    // 首先就是K线图priceDisplayWidth
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

    int mainContentWidth = event->rect().width() - PRICE_AREA_WIDTH;
    QRect mainRect(event->rect());
    mainRect.setWidth(mainContentWidth);
    judgeDisplay(mainRect, kInfo);
    int displayNum = mainContentWidth / eachLineWidth;
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
        QLine tempLine(0, eachWinHeight * i, mainContentWidth, eachWinHeight * i);
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

    // 最右侧绘制一个显示栏，用于显示价格(分成四个等份)
    QFont priceFont;
    priceFont.setPointSize(FONT_SIZE);
    painter->setFont(priceFont);
    QLine rightEndLine(mainContentWidth, 0, mainContentWidth, event->rect().height());
    painter->drawLine(rightEndLine);

    int eachPriceLevelHeight = event->rect().height() / 4;
    for(int i = 0;i < 4;i++) {
        float curPrice = maxPrice - minMaxDelta *
                (FONT_SIZE + eachPriceLevelHeight * i) / event->rect().height();
        painter->drawText(mainContentWidth + PRICE_MARGIN_LEFT,
                          FONT_SIZE + eachPriceLevelHeight * i,
                          QString::fromStdString(std::to_string(curPrice).substr(0, 5)));
    }

    // 绘制一下当前鼠标所在K线的位置，加一个十字线，贯穿整个窗口
    int kLineNum = currMouseP.x() / eachLineWidth;
    int pointX = 0;
    // 处理一下，避免垂直的线越过K线显示区，到了右边价格显示区
    if(kLineNum > (endIndex - startIndex)) {
        pointX = mainContentWidth;
    }
    else {
        pointX = kLineNum * eachLineWidth + eachLineWidth / 2;
    }
    QLine horizonLine(0, currMouseP.y(), mainContentWidth, currMouseP.y());
    painter->drawLine(horizonLine);

    QLine verticLine(pointX, 0, pointX, event->rect().height());
    painter->drawLine(verticLine);

    // 绘制一下当前的价格
    int priceBackGroudHeight = FONT_SIZE + 4;
    float mouseOnPrice = maxPrice - minMaxDelta * currMouseP.y() / event->rect().height();
    QRect mouseOnPriceBack(mainContentWidth, currMouseP.y() - priceBackGroudHeight + 2, PRICE_AREA_WIDTH, priceBackGroudHeight);
    painter->drawRect(mouseOnPriceBack);
    QRect priceFillBack(mainContentWidth + 1, currMouseP.y() - priceBackGroudHeight + 3, PRICE_AREA_WIDTH - 2, priceBackGroudHeight - 2);
    painter->fillRect(priceFillBack, QColor(184, 243, 144));
    painter->drawText(mainContentWidth + PRICE_MARGIN_LEFT, currMouseP.y(),
                      QString::fromStdString(std::to_string(mouseOnPrice).substr(0, 5)));
}

void StockChart::paintIndexLine(QPainter* painter, QPaintEvent *event) {
    boost::chrono::system_clock::time_point curr = boost::chrono::system_clock::now();
    QPen pen;
    QColor color;
    color.setRgb(217, 58, 24);
    pen.setColor(color);
    pen.setWidth(2);
    painter->setPen(pen);
    painter->setRenderHint(QPainter::Antialiasing);

    const StockIndexBatchInfo& indexInfo = *model->getCurrStockIndexInfo();

    // 沪深两市总交易时长（单位：秒）
    int totalSeconds = 4 * 60 * 60;

    // 开市时间
    QString today = QDateTime::currentDateTime().toString("yyyy-MM-dd");
    const QDateTime morningStartTime = QDateTime::fromString(today + " 09:30:00",
                                                            "yyyy-MM-dd HH:mm:ss");
    const QDateTime morningEndTime = QDateTime::fromString(today + " 11:30:00",
                                                            "yyyy-MM-dd HH:mm:ss");
    const QDateTime afternoonStartTime = QDateTime::fromString(today + " 13:00:00",
                                                                "yyyy-MM-dd HH:mm:ss");
    const QDateTime afternoonEndTime = QDateTime::fromString(today + " 15:00:00",
                                                                "yyyy-MM-dd HH:mm:ss");

    // 前一天的收盘价
    double preDayClose = indexInfo.pre_close;
    size_t lastIndex=  indexInfo.info_list.size() - 1;
    double minPrice = indexInfo.info_list.size() > 0 ? indexInfo.info_list[lastIndex].
            mainContent[StockIndexBatchInfo::CURR_MIN] : 0;
    double maxPrice = indexInfo.info_list.size() > 0 ? indexInfo.info_list[lastIndex].
            mainContent[StockIndexBatchInfo::CURR_MAX] : 0;
    double absMax = fabs(minPrice - preDayClose) > fabs(maxPrice - preDayClose)
            ? minPrice : maxPrice;
    double max_pct = fabs(absMax - preDayClose) / preDayClose * 100;
    double delta = 2 * fabs(absMax - preDayClose);
    // 将最大波动百分比均分成６份，然后上下对等来画
    double per_pct = max_pct / 6;
    //　分时图主体显示区宽度
    int mainContentWidth = event->rect().width() - PRICE_AREA_WIDTH;

    // 然后绘制右边的百分比显示区以及窗格系统
    QFont priceFont;
    priceFont.setPointSize(FONT_SIZE);
    painter->setFont(priceFont);
    QLine rightEndLine(mainContentWidth, 0, mainContentWidth, event->rect().height());
    painter->drawLine(rightEndLine);

    int eachPriceLevelHeight = event->rect().height() / 13;
    //　绘制窗格系统，　重置下画笔颜色
    QColor lineColor;
    lineColor.setRgb(206, 200, 200);
    pen.setColor(lineColor);
    painter->setPen(pen);
    for(int i = 0;i < 13;i++) {
        QLine tempLine(0, eachPriceLevelHeight * i, mainContentWidth,
                       eachPriceLevelHeight * i);
        painter->drawLine(tempLine);
    }

    //　绘制涨跌幅百分显示区
    pen.setColor(color);
    painter->setPen(pen);
    for(int i = 0;i < 13;i++) {
        double currPct = max_pct - per_pct * i;
        painter->drawText(mainContentWidth + PRICE_MARGIN_LEFT,
                          FONT_SIZE + eachPriceLevelHeight * i,
                          QString::fromStdString(std::to_string(currPct).
                                                 substr(0, 5)).append("%"));
    }

    // 绘制一下当前鼠标所在K线的位置，加一个十字线，贯穿整个窗口
    // 处理一下，避免垂直的线越过K线显示区，到了右边价格显示区
    int pointX = currMouseP.x() > mainContentWidth ? mainContentWidth : currMouseP.x();
    QLine horizonLine(0, currMouseP.y(), mainContentWidth, currMouseP.y());
    painter->drawLine(horizonLine);

    QLine verticLine(pointX, 0, pointX, event->rect().height());
    painter->drawLine(verticLine);

    // 绘制一下当前的价格百分比
    int priceBackGroudHeight = FONT_SIZE + 4;
    int halfHeight = event->rect().height() / 2;
    int currMouseOnY = currMouseP.y();
    float mouseOnPct = std::abs(currMouseOnY - halfHeight) * max_pct / halfHeight;
    mouseOnPct = currMouseOnY > halfHeight ? -mouseOnPct : mouseOnPct;
    QRect mouseOnPriceBack(mainContentWidth, currMouseP.y() - priceBackGroudHeight + 2, PRICE_AREA_WIDTH, priceBackGroudHeight);
    painter->drawRect(mouseOnPriceBack);
    QRect priceFillBack(mainContentWidth + 1, currMouseP.y() - priceBackGroudHeight + 3, PRICE_AREA_WIDTH - 2, priceBackGroudHeight - 2);
    painter->fillRect(priceFillBack, QColor(184, 243, 144));
    painter->drawText(mainContentWidth + PRICE_MARGIN_LEFT, currMouseP.y(),
                      QString::fromStdString(std::to_string(mouseOnPct).
                                             substr(0, 5)).append('%'));

    // 最后绘制主体部分
    int preX = -1, preY = -1; // 上一根线条的结束点
    QDateTime preDateTime = indexInfo.info_list.size() > 0 ? indexInfo.info_list[0].time :
                                                             QDateTime::currentDateTime();
    for(size_t i = 1;i < indexInfo.info_list.size();i++) {
        double currPrice = indexInfo.info_list[i].mainContent[StockIndexBatchInfo::CURR_PRICE];
        double toTop = currPrice < preDayClose ?
                    preDayClose - currPrice + fabs(absMax - preDayClose) :
                    fabs(absMax - currPrice);
        int y = static_cast<int>(toTop / delta * event->rect().height());

        // 规定一个间隔时间，小于间隔时间之内的点可以不必重复绘制，因为比较卡
        // 但是除了最后一个点
        if(preDateTime.secsTo(indexInfo.info_list[i].time) < INDEX_TWO_POINT_TIME_DELTA
                && i < indexInfo.info_list.size() - 1) {
            continue;
        }
        // 计算一下经历了多长的交易时间(午休时间不计算在内)
        int deltaTime = 0;
        // 上午交易时间段内
        if(indexInfo.info_list[i].time > morningStartTime &&
                indexInfo.info_list[i].time < morningEndTime) {
            deltaTime += morningStartTime.secsTo(indexInfo.info_list[i].time);
        }
        // 下午交易时间段内
        else if(indexInfo.info_list[i].time > afternoonStartTime &&
                indexInfo.info_list[i].time < afternoonEndTime){
            deltaTime += totalSeconds / 2 + afternoonStartTime.secsTo(indexInfo.info_list[i].time);
        }
        else {
            continue;
        }

        int x = static_cast<int>(deltaTime / static_cast<double>(totalSeconds)
                                 * mainContentWidth);
        if(preX == -1) {
            //　第一个点，不用画了
            preX = x;
            preY = y;
            continue;
        }
        QLine tempLine(preX, preY, x, y);
        painter->drawLine(tempLine);
        preX = x;
        preY = y;

        preDateTime = indexInfo.info_list[i].time;
    }

    boost::chrono::system_clock::time_point curr2 = boost::chrono::system_clock::now();
    boost::chrono::system_clock::duration dur = curr2 - curr;
    boost::chrono::microseconds relDur = boost::chrono::duration_cast<boost::chrono::microseconds>(dur);
    //std::cout << "fetch data time : " << relDur << std::endl;

}

// FIXME -- 　鼠标移动在分时图没有数据的时候会导致界面卡死
void StockChart::mouseMoveEvent(QMouseEvent *event) {
    currMouseP = event->pos();
    update();

    // 发送一个事件，通知一下光标所在位置的股票已经变动了
    if(currDisplayType == DisplayType::K_TYPE) {
        // 计算一下当前光标位置下的股票索引
        int kLineNum = currMouseP.x() / eachLineWidth;
        StockInfo currMouseOnInfo;
        StockBatchInfo* kInfo = model->getCurrStockKInfo();
        currMouseOnInfo.ts_code = kInfo->getTsCode();
        currMouseOnInfo.ts_name = kInfo->getTsName();
        int currMouseOnIndex = startIndex + kLineNum;
        // 因为右边有价格显示区，所以要避免数组越界的问题
        if(currMouseOnIndex >= kInfo->info_list.size()) {
            return;
        }
        // 从Model当中获取相应的股票信息
        StockBatchInfo::SingleInfo originInfo = kInfo->info_list[currMouseOnIndex];
        currMouseOnInfo.trade_date = originInfo.tradeDate;
        currMouseOnInfo.low = originInfo.low;
        currMouseOnInfo.high = originInfo.high;
        currMouseOnInfo.open = originInfo.open;
        currMouseOnInfo.close = originInfo.close;
        currMouseOnInfo.pct_chg = originInfo.pct_chg;
        currMouseOnInfo.trade_date = originInfo.tradeDate;

        // 获取一下前一天的价格
        currMouseOnInfo.pre_close = currMouseOnIndex > 0 ?
                    kInfo->info_list[currMouseOnIndex - 1].close : 0;
        // 发送相应的事件
        emit mouseOnChanged(currMouseOnInfo);
    }
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


void StockChart::resizeEvent(QResizeEvent *) {
    isFirstRender = true;
    update();
}

