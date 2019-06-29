#include "stockinfo.h"

StockBatchInfo::StockBatchInfo(StockBatchInfo&& origin) noexcept {
    this->ts_code = std::move(origin.ts_code);
    this->ts_name = std::move(origin.ts_name);
    this->info_list = std::move(origin.info_list);
}

StockBatchInfo& StockBatchInfo::operator=(StockBatchInfo&& origin) noexcept {
    this->ts_code = std::move(origin.ts_code);
    this->ts_name = std::move(origin.ts_name);
    this->info_list = std::move(origin.info_list);
}

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
