#include "stockchartmodel.h"
#include <QModelIndex>
#include <iostream>
#include "data/datacenter.h"
#include "data/stockindexinfo.h"

StockChartModel::StockChartModel() {
    DataCenter& dataCenter = DataCenter::getInstance();
    currSelectedKInfo = dataCenter.getStockBatchInfoByTsCode("000001.SZ");
    currSelectedIndexInfo = dataCenter.getStockIndexInfo("000001.SZ");
}

void StockChartModel::setSelectedStock(std::string ts_code) noexcept {
    if(ts_code != currSelectedTsCode) {
        DataCenter& dataCenter = DataCenter::getInstance();
        std::cout << "curr selected ts is : " << ts_code << std::endl;
        currSelectedTsCode = ts_code;
        currSelectedKInfo = dataCenter.getStockBatchInfoByTsCode(QString::fromStdString(ts_code));
        reset();
    }
}

void StockChartModel::currSelectdStockChanged(const QModelIndex& index) {
    DataCenter& dataCenter = DataCenter::getInstance();
    QVariant ts_code = index.data(CommonConst::PRIMARY_KEY_ROLE);
    QString real_code = ts_code.toString();
    currSelectedTsCode = real_code.toStdString();
    currSelectedKInfo = dataCenter.getStockBatchInfoByTsCode(real_code);
    reset();
}

void StockChartModel::reset() {
    emit dataChanged();
}

StockBatchInfo* StockChartModel::getCurrStockKInfo() {
    return currSelectedKInfo;
}

// TODO --此处可能有效率问题？毕竟拷贝的数据量有点大
StockIndexBatchInfo StockChartModel::getCurrStockIndexInfo() {
    DataCenter& instance = DataCenter::getInstance();
    currSelectedIndexInfo = instance.getStockIndexInfo(currSelectedTsCode);
    return currSelectedIndexInfo;
}
