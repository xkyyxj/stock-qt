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
        singleInfo.pct_chg = queryInfo.value("pct_chg").toFloat();
        // TODO -- 下面这一行似乎是不能正常工作的，所以需要修正一下
        //singleInfo.tradeDate = queryInfo.value("trade_date").toDate();
        QString dateStr = queryInfo.value("trade_date").toString();
        singleInfo.tradeDate = QDate::fromString(dateStr, "yyyyMMdd");
        info_list.push_back(std::move(singleInfo));
    }
}
