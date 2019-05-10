#include "stockchartmodel.h"


StockChartModel::StockChartModel(DataCenter& data_center): dataCenter(data_center) {
    currSelectedKInfo = nullptr;

}


void StockChartModel::currSelectdStockChanged(QString ts_code) {
    currSelectedKInfo = dataCenter.getStockBatchInfoByTsCode(ts_code);
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
