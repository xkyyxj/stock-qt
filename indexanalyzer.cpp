#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "data/datacenter.h"
#include "indexanalyzer.h"
#include "data/stockbaseinfo.h"

IndexAnalyzer::IndexAnalyzer() noexcept {
    // 生成一个唯一的UUID，作为数据库连接的标识符(避免同DataCenter当中的默认的数据库连接重名)
    boost::uuids::uuid temp_uuid = boost::uuids::random_generator()();
    std::string uuidStr = boost::uuids::to_string(temp_uuid);
    defaultDatabase = QSqlDatabase::addDatabase("QMYSQL", QString::fromStdString(uuidStr));
    defaultDatabase.setHostName("localhost");
    defaultDatabase.setDatabaseName("stock");
    defaultDatabase.setUserName("root");
    defaultDatabase.setPassword("123");
    defaultDatabase.open();
}

[[noreturn]] void IndexAnalyzer::operator()() noexcept {
    DataCenter& instance = DataCenter::getInstance();

    //　查询所有的列表(TODO -- 不能够动态的获取变更的列表)
    std::vector<StockBaseInfo> stockList = instance.getStockList(defaultDatabase);
    for(;;) {
        for(size_t i = 0;i < stockList.size();i++) {
            StockBaseInfo currInfo = stockList[i];
            QString ts_code = currInfo.ts_code;
            StockIndexBatchInfo retVal = instance.getStockIndexInfo(ts_code.toStdString());
        }
    }
}

/**
 * 判定日线图上快速上涨的
 * 上涨百分比/所用时间(秒)
 * 入选阈值：> 1%/60秒
 */
bool judgeQuickUp(StockIndexBatchInfo& info) noexcept {
    size_t infoSize = info.info_list.size();
    if(infoSize < 2)
        return false;

    // 获取最后两个时间点的日线数据
    StockIndexBatchInfo::SingleIndexInfo lastPoint = info.info_list[infoSize - 1];
    StockIndexBatchInfo::SingleIndexInfo lastPrePoint = info.info_list[infoSize - 2];

    qint64 secondsDelta = lastPrePoint.time.secsTo(lastPoint.time);

}

/**
 * TODO -- 低于收盘价格的急速下跌后反弹
 * @return 返回一个分数，代表买入价值
 */
int quickDownThenUp(StockIndexBatchInfo& info) noexcept {
    QDateTime currTime = QDateTime::currentDateTime();
    QDateTime fortyMinBefore = currTime.addSecs(-60 * 40);
    std::vector<StockIndexBatchInfo::SingleIndexInfo>& v = info.info_list;
    double preDayClose = info.pre_close;
    double toayOpen = info.today_open;
    bool targetDown = false;
    double maxDownPct = 0;
    for(size_t i = 0;i < v.size();i++) {
        if(v[i].time >= fortyMinBefore && !targetDown) {
            double p = v[i].mainContent[StockIndexBatchInfo::CURR_PRICE];
            double chg_pct = (p - toayOpen) / toayOpen;
            maxDownPct = chg_pct < maxDownPct ? chg_pct : maxDownPct;
            if(chg_pct < -0.06 && chg_pct > -0.08) {
                chg_pct = (p - preDayClose) / preDayClose;
                if(chg_pct < -0.04) {
                    targetDown = true;
                }
            }
        }
    }

    // 判定一下最新一条记录，是否是反转行情
    if(targetDown) {
        size_t lastIndex = v.size() - 1;
        double p = v[lastIndex].mainContent[StockIndexBatchInfo::CURR_PRICE];
        double chg = (p - preDayClose) / preDayClose;
        if((chg - maxDownPct) > 0.01) {
            return 1;
        }
    }
}
