#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <iostream>
#include "data/datacenter.h"
#include "indexanalyzer.h"
#include "data/stockbaseinfo.h"

// 两次实时数据分析之间的时间价格不少于2000毫秒
static const long TWO_ANA_DELTA_TIME = 2000;

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

IndexAnalyzer::IndexAnalyzer(const IndexAnalyzer& origin) noexcept {
    defaultDatabase = origin.defaultDatabase;
    cacheTools = RedisCacheTools();
}

/**
 * 最终决定将实时信息的分析结果放入到redis缓存当中
 */
[[noreturn]] void IndexAnalyzer::operator()() noexcept {
    using namespace boost::this_thread;
    using namespace boost::chrono;

    struct QuickUp {
        QString ts_code;
        double up_rate;
    };

    DataCenter& instance = DataCenter::getInstance();
    // 轮询间隔控制变量
    boost::chrono::system_clock::time_point p = boost::chrono::system_clock::now();
    milis_t lastAnaTime = boost::chrono::time_point_cast<boost::chrono::milliseconds>(p);

    std::vector<QuickUp> quick_up_rst;
    std::vector<std::string> index_down_up_rst;

    //　查询所有的列表(TODO -- 不能够动态的获取变更的列表)
    std::vector<StockBaseInfo> stockList = instance.getStockList(defaultDatabase);
    for(;;) {

        milis_t currTime = time_point_cast<milliseconds>(system_clock::now());
        milliseconds time_delta = currTime - lastAnaTime;
        // 两次获取股票实时数据之间的时间间隔不少于2000毫秒
        if(time_delta.count() < TWO_ANA_DELTA_TIME) {
            long long target_count = TWO_ANA_DELTA_TIME - time_delta.count();
            milliseconds target_delta(target_count);
            currTime += target_delta;
        }
        sleep_until(currTime);

        milis_t time2 = time_point_cast<milliseconds>(system_clock::now());
        for(size_t i = 0;i < stockList.size();i++) {
            StockBaseInfo currInfo = stockList[i];
            QString ts_code = currInfo.ts_code;
            StockIndexBatchInfo retVal = instance.getStockIndexInfo(ts_code.toStdString());
            double up_rate = 0;
//            if(judgeQuickUp(retVal, up_rate)) {
//                QuickUp quick_up;
//                quick_up.ts_code = retVal.ts_code;
//                //quick_up.ts_name = retVal.ts_name;
//                quick_up.up_rate = up_rate;
//                //quick_up.datetime = QDateTime::currentDateTime();
//                quick_up_rst.push_back(quick_up);
//            }

//            if(quickDownThenUp(retVal)) {
//                IndexDownUp i_down_up;
//                i_down_up.ts_code = retVal.ts_code;
//                i_down_up.ts_name = retVal.ts_name;
//                i_down_up.datetime = QDateTime::currentDateTime();
//            }
        }

        analyzeLastMaxUp();
        milis_t anaEndTime = time_point_cast<milliseconds>(system_clock::now());
        milliseconds timeDelta = anaEndTime - time2;
        std::cout << "ana time cast: " << duration_cast<seconds>(timeDelta) <<
                     std::endl;
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
    return -1;
}

/**
 * 昨日涨停今日日线图上表现还算OK的:
 * 昨日涨停，并且今天在分时图上开启上涨模式：上涨速率大于等于1%每分钟
 * 存入redis当中，key为：lmu_ok
 */
void IndexAnalyzer::analyzeLastMaxUp() noexcept {
    DataCenter& instance = DataCenter::getInstance();

    //　查询出昨日涨停的股票
    std::vector<QString> last_max_up;
    QString sql("select ts_code from last_max_up");
    instance.executeQuery(sql, [&last_max_up](QSqlQuery& query) -> void {
        while(query.next()) {
            last_max_up.push_back(query.value("ts_code").toString());
        }
    },defaultDatabase);

    std::vector<std::string> final_rst;
    for(size_t i = 0;i < last_max_up.size();i++) {
        StockIndexBatchInfo retVal = instance.getStockIndexInfo(
                    last_max_up[i].toStdString());
        // 开始上涨
        if(retVal.info_list.size() > 2) {
            size_t lastIndex=  retVal.info_list.size() - 1;
            double lastPrePrice = retVal.info_list[lastIndex - 1].mainContent[StockIndexBatchInfo::CURR_PRICE];
            double lastPrice = retVal.info_list[lastIndex].mainContent[StockIndexBatchInfo::CURR_PRICE];
            double currMax = retVal.info_list[lastIndex].mainContent[StockIndexBatchInfo::CURR_MAX];
            double up_pct = (lastPrice - lastPrePrice) / lastPrePrice;
            double timeDelta = retVal.info_list[lastIndex - 1].time.secsTo(
                        retVal.info_list[lastIndex].time);
            // 每分钟上涨的百分比
            double up_rate = timeDelta > 0 ? up_pct / timeDelta * 60 : 0;
            // 最新价格是最高价，并且上涨速率大于等于1%每分钟
            if(lastPrice - currMax < 0.01 && up_rate >= 0.01) {
                final_rst.push_back(retVal.ts_code.toStdString());
            }
        }
    }

    if(final_rst.size() > 0) {
        cacheTools.delKey("lmu_ok");
        cacheTools.pushBackToList("lmu_ok", final_rst);
    }
}

/**
 * 寻找快速上涨的
 */
void IndexAnalyzer::findQuickUp(std::vector<QString>& stockList) noexcept {

}
