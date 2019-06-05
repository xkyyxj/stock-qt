#include "stockchartmodel.h"
#include <QModelIndex>
#include <iostream>

StockChartModel::StockChartModel(DataCenter* data_center): dataCenter(data_center) {
    currSelectedKInfo = dataCenter->getStockBatchInfoByTsCode("000001.SZ");

}


void StockChartModel::currSelectdStockChanged(const QModelIndex& index) {
    QVariant ts_code = index.data(CommonConst::PRIMARY_KEY_ROLE);
    QString real_code = ts_code.toString();
    currSelectedKInfo = dataCenter->getStockBatchInfoByTsCode(real_code);
    reset();
}

void StockChartModel::reset() {
    emit dataChanged();
}

StockBatchInfo* StockChartModel::getCurrStockKInfo() {
    return currSelectedKInfo;
}
StockIndexBatchInfo* StockChartModel::getCurrStockIndexInfo() {
    return currSelectedIndexInfo;
}
