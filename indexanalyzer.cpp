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

    struct QuickUp {
        QString ts_code, ts_name;
        QDateTime datetime;
        double up_rate;
    };

    struct IndexDownUp {
        QString ts_code, ts_name;
        QDateTime datetime;
    };

    DataCenter& instance = DataCenter::getInstance();

    std::vector<QuickUp> quick_up_rst;
    std::vector<IndexDownUp> index_down_up_rst;

    //　查询所有的列表(TODO -- 不能够动态的获取变更的列表)
    std::vector<StockBaseInfo> stockList = instance.getStockList(defaultDatabase);
    for(;;) {
        for(size_t i = 0;i < stockList.size();i++) {
            StockBaseInfo currInfo = stockList[i];
            QString ts_code = currInfo.ts_code;
            StockIndexBatchInfo retVal = instance.getStockIndexInfo(ts_code.toStdString());
            double up_rate = 0;
            if(judgeQuickUp(retVal, up_rate)) {
                QuickUp quick_up;
                quick_up.ts_code = retVal.ts_code;
                quick_up.ts_name = retVal.ts_name;
                quick_up.up_rate = up_rate;
                quick_up.datetime = QDateTime::currentDateTime();
                quick_up_rst.push_back(quick_up);
            }

            if(quickDownThenUp(retVal)) {
                IndexDownUp i_down_up;
                i_down_up.ts_code = retVal.ts_code;
                i_down_up.ts_name = retVal.ts_name;
                i_down_up.datetime = QDateTime::currentDateTime();
            }
        }

        //　将结果插入到数据库当中
        std::vector<std::string> columns;
        columns.push_back("ts_code");
        columns.push_back("ts_name");
        columns.push_back("datetime");
        columns.push_back("up_rate");
        std::vector<QVariantList*> insertParams;
        QVariantList codeList;
        QVariantList nameList;
        QVariantList datetimeList;
        QVariantList upRateList;
        for (QuickUp& temp : quick_up_rst) {
            codeList << temp.ts_code;
            nameList << temp.ts_name;
            datetimeList << temp.datetime;
            upRateList << temp.up_rate;
            insertParams.push_back(&codeList);
            insertParams.push_back(&nameList);
            insertParams.push_back(&datetimeList);
            insertParams.push_back(&upRateList);
            instance.executeInsert("index_quick_up", columns, insertParams, defaultDatabase);
        }

        columns.clear();
        columns.push_back("ts_code");
        columns.push_back("ts_name");
        columns.push_back("datetime");
        insertParams.clear();
        codeList.clear();
        nameList.clear();
        datetimeList.clear();
        for (IndexDownUp& temp : index_down_up_rst) {
            codeList << temp.ts_code;
            nameList << temp.ts_name;
            datetimeList << temp.datetime;
            insertParams.push_back(&codeList);
            insertParams.push_back(&nameList);
            insertParams.push_back(&datetimeList);
            instance.executeInsert("index_down_up", columns, insertParams, defaultDatabase);
        }
    }
}

/**
 * 判定日线图上快速上涨的
 * 上涨百分比/所用时间(秒)
 * 入选阈值：> 1%/60秒
 */
bool IndexAnalyzer::judgeQuickUp(StockIndexBatchInfo& info, double& ret_up_rate) noexcept {
    size_t infoSize = info.info_list.size();
    if(infoSize < 2)
        return false;

    // 获取最后两个时间点的日线数据
    StockIndexBatchInfo::SingleIndexInfo lastPoint = info.info_list[infoSize - 1];
    StockIndexBatchInfo::SingleIndexInfo lastPrePoint = info.info_list[infoSize - 2];

    qint64 secondsDelta = lastPrePoint.time.secsTo(lastPoint.time);
    if(secondsDelta > 0) {
        // 计算一下涨跌百分比
        double lastPrice = lastPoint.mainContent[StockIndexBatchInfo::CURR_PRICE];
        double prePrice = lastPrePoint.mainContent[StockIndexBatchInfo::CURR_PRICE];
        double pct = (lastPrice - prePrice) / prePrice;
        double up_rate = pct / secondsDelta * 60;
        ret_up_rate = up_rate;
        return up_rate > 0.01;
    }
    return false;
}

/**
 * TODO -- 低于收盘价格的急速下跌后反弹
 * @return 返回一个分数，代表买入价值
 */
int IndexAnalyzer::quickDownThenUp(StockIndexBatchInfo& info) noexcept {
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
