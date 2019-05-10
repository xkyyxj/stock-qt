#include "stockinfo.h"

void StockBatchInfo::addSingleDayInfos(QSqlQuery& queryInfo) {
    while(queryInfo.next()) {
        SingleInfo singleInfo;
        singleInfo.low = queryInfo.value("low").toFloat();
        singleInfo.high = queryInfo.value("high").toFloat();
        singleInfo.open = queryInfo.value("open").toFloat();
        singleInfo.close = queryInfo.value("close").toFloat();
        singleInfo.tradeDate = queryInfo.value("trade_date").toDate();
        info_list.push_back(std::move(singleInfo));
    }
}
