#pragma execution_character_set("utf-8")
#ifndef STOCKCHART_H
#define STOCKCHART_H

#include <QList>

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QFrame>
#include "data/stockinfo.h"
#include "data/stockindexinfo.h"
#include "stockchartmodel.h"

class StockChart : public QFrame
{
    Q_OBJECT

public:
    enum DisplayType {
        K_TYPE, INDEX_TYPE
    };
    StockChart(QWidget *parent);

    ~StockChart() override;

    void setModel(StockChartModel*);

public slots:
    void stockInfoChanged();

protected:
    void paintEvent(QPaintEvent *event) override;
    void paintKLine(QPainter* painter, QPaintEvent *event);
    void paintIndexLine(QPainter* painter, QPaintEvent *event);

    void mouseMoveEvent(QMouseEvent *event) override;
    void keyReleaseEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    static const int MIN_K_LINE_WIDTH;
    static const int DEFAULT_K_LINE_WIDTH;
    static const int MAX_K_LINE_WIDTH;
    void judgeDisplay(const QRect& rect, StockBatchInfo* kInfo);

private:
    QList<StockInfo> kList;
    DisplayType currDisplayType;
    StockChartModel* model;

    // 每根K线的宽度
    int eachLineWidth;

    // 显示的K线区间，根据index值来判定
    int startIndex, endIndex;

    // 是否是第一次加载界面，每当要显示的K线有变动的时候，都会将这个变量重置为true
    bool isFirstRender;

    // 当前鼠标的位置
    QPoint currMouseP;
};
//! [0]

#endif
