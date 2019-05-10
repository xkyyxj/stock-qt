#ifndef STOCKCHART_H
#define STOCKCHART_H

#include <QList>

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QOpenGLWidget>
#include "stockinfo.h"
#include "stockindexinfo.h"
#include "stockchartmodel.h"

class StockChart : public QOpenGLWidget
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

private:
    QList<StockInfo> kList;
    DisplayType currDisplayType;
    StockChartModel* model;
};
//! [0]

#endif
