#pragma execution_character_set("utf-8")
#ifndef STOCKCHARTMODEL_H
#define STOCKCHARTMODEL_H

#include <QObject>
#include "stockinfo.h"
#include "stockindexinfo.h"
#include "commonenum.h"

class StockChartModel : public QObject{
    Q_OBJECT
public:
    StockChartModel();

    StockBatchInfo* getCurrStockKInfo();
    StockIndexBatchInfo* getCurrStockIndexInfo();
public slots:
    void currSelectdStockChanged(const QModelIndex& index);

signals:
    void dataChanged();

private:
    StockBatchInfo* currSelectedKInfo;
    StockIndexBatchInfo* currSelectedIndexInfo;

    void reset();

};

#endif // STOCKCHARTMODEL_H
