#pragma execution_character_set("utf-8")
#ifndef STOCKCHARTMODEL_H
#define STOCKCHARTMODEL_H

#include <QObject>
#include "data/stockinfo.h"
#include "data/stockindexinfo.h"
#include "commonenum.h"

class StockChartModel : public QObject{
    Q_OBJECT
public:
    StockChartModel();

    StockBatchInfo* getCurrStockKInfo();
    const StockIndexBatchInfo* getCurrStockIndexInfo();

    void setSelectedStock(std::string ts_code) noexcept;
public slots:
    void currSelectdStockChanged(const QModelIndex& index);

signals:
    void dataChanged();

private:
    std::string currSelectedTsCode = "000001.SZ"; //　给个默认值
    StockBatchInfo* currSelectedKInfo;
    StockIndexBatchInfo currSelectedIndexInfo;

    void reset();

};

#endif // STOCKCHARTMODEL_H
