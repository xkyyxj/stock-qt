#ifndef STOCKCHARTMODEL_H
#define STOCKCHARTMODEL_H

#include <QObject>
#include "stockinfo.h"
#include "stockindexinfo.h"
#include "datacenter.h"

class StockChartModel : public QObject{
    Q_OBJECT
public:
    StockChartModel(DataCenter&);

    StockBatchInfo* getCurrStockKInfo();
    StockIndexBatchInfo* getCurrStockIndexInfo();

private slots:
    void currSelectdStockChanged(QString ts_code);

signals:
    void dataChanged();

private:
    StockBatchInfo* currSelectedKInfo;
    StockIndexBatchInfo* currSelectedIndexInfo;
    DataCenter& dataCenter;

    void reset();

};

#endif // STOCKCHARTMODEL_H
