#ifndef STOCKCHARTMODEL_H
#define STOCKCHARTMODEL_H

#include <QObject>
#include "stockinfo.h"
#include "stockindexinfo.h"
#include "datacenter.h"
#include "commonenum.h"

class StockChartModel : public QObject{
    Q_OBJECT
public:
    StockChartModel(DataCenter*);

    StockBatchInfo* getCurrStockKInfo();
    StockIndexBatchInfo* getCurrStockIndexInfo();
public slots:
    void currSelectdStockChanged(const QModelIndex& index);

signals:
    void dataChanged();

private:
    StockBatchInfo* currSelectedKInfo;
    StockIndexBatchInfo* currSelectedIndexInfo;
    DataCenter* dataCenter;

    void reset();

};

#endif // STOCKCHARTMODEL_H
