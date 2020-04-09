#include <QString>

#include "calculator.h"
#include "data/datacenter.h"
#include <boost/thread.hpp>
#include "data/stockbaseinfo.h"
#include <QAction>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <iostream>

Calculator::Calculator() noexcept {
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

Calculator::Calculator(const Calculator& origin) noexcept {
    // 生成一个唯一的UUID，作为数据库连接的标识符(避免同DataCenter当中的默认的数据库连接重名)
    // 为什么这儿就可以用非本线程创建的QSqlDatabase?
    this->defaultDatabase = origin.defaultDatabase;
    /*boost::uuids::uuid temp_uuid = boost::uuids::random_generator()();
    std::string uuidStr = boost::uuids::to_string(temp_uuid);
    defaultDatabase = QSqlDatabase::addDatabase("QMYSQL", QString::fromStdString(uuidStr));
    defaultDatabase.setHostName("localhost");
    defaultDatabase.setDatabaseName("stock");
    defaultDatabase.setUserName("root");
    defaultDatabase.setPassword("123");
    defaultDatabase.open();*/
}

void Calculator::initData() noexcept {
    DataCenter& instance = DataCenter::getInstance();
    stockList = instance.getStockList(defaultDatabase);
}

void Calculator::operator()() noexcept {
    initData();
//    findVWaveStock(3);
//    findContinueUpStock(3);
//    findBigWave();
//    lastDayMaxUp();
//    //　寻找已经大幅下跌的股票
//    findBigDown();

//    findBigDownThenUp();
    findInLowPrice();
    std::cout << "all calcaulate is finished" << std::endl;
}

/**
 *  查找V型反转，经历了若干天的下跌之后开始第一根阳线
 */
void Calculator::findVWaveStock(int checkDays) noexcept {

    struct VWave {
        QString ts_code, ts_name;
    };

    DataCenter& instance = DataCenter::getInstance();
    boost::chrono::system_clock::time_point now = boost::chrono::system_clock::now();

    // 计算之前首先先把要生成天的数据删掉
    QDate date = QDate::currentDate();
    QString dateStr = date.toString("yyyy-MM-dd");
    QString delWhere(" date='");
    delWhere.append(dateStr).append("'");
    instance.executeDel("v_wave", delWhere, defaultDatabase);

    // 下降过程当中必须是阴线，并且最后一根阳线收盘价高于前一天开盘价
    std::vector<VWave> downGreenUpHigh;
    // 下降过程当中必须是阴线，并且最后一根阳线收盘价可低于前一天开盘价
     std::vector<VWave> downGreenUpLow;
    // 下降过程当中可以是阳线或阴线，并且最后一根阳线收盘价高于前一天开盘价
    std::vector<VWave> downRedUpHigh;
    // 下降过程当中可以是阳线或阴线，并且最后一根阳线收盘价可低于前一天开盘价
    std::vector<VWave> downRedUpLow;
    std::vector<VWave> finalResult;

    for(size_t i = 0;i < stockList.size();i++) {
        QString ts_code = stockList[i].ts_code;
        StockBatchInfo dayInfo = instance.getStockDayInfo(ts_code.toStdString(), defaultDatabase);

        bool is_continue_down = true, downGreen = false;
        int lastIndex = dayInfo.info_list.size() - 1;
        if(dayInfo.info_list.size() > checkDays) {
            for(int j = 1;j < checkDays;j++) {
                StockBatchInfo::SingleInfo singleInfo = dayInfo.info_list[lastIndex - j];
                is_continue_down = singleInfo.pct_chg < 0;
                downGreen = singleInfo.open - singleInfo.close < -0.01f;
                if(!is_continue_down) {
                    break;
                }
            }
        }
        else {
            is_continue_down = false;
        }

        if(is_continue_down) {
            StockBatchInfo::SingleInfo preDayInfo = dayInfo.info_list[lastIndex - 1];
            StockBatchInfo::SingleInfo info = dayInfo.info_list[lastIndex];
            VWave tempInfo;
            tempInfo.ts_code = ts_code;
            tempInfo.ts_name = dayInfo.ts_name;
            // 最后一天是阳线
            if(info.close > info.open) {
                if(downGreen) {
                    if(info.close - preDayInfo.open > 0.001f) {
                        downGreenUpHigh.push_back(tempInfo);
                    }
                    else {
                        downGreenUpLow.push_back(tempInfo);
                    }
                }
                else {
                    if(info.close - preDayInfo.open > 0.001f) {
                        downRedUpHigh.push_back(tempInfo);
                    }
                    else {
                        downRedUpLow.push_back(tempInfo);
                    }
                }
            }
        }
    }

    std::vector<VWave>* tempArray[4] = {&downGreenUpHigh, &downGreenUpLow,
                                          &downRedUpLow, &downRedUpHigh};
    for(std::vector<VWave>* tempVect : tempArray) {
        if(tempVect->size() > 0) {
            std::vector<std::string> columns;
            columns.push_back("ts_code");
            columns.push_back("ts_name");
            columns.push_back("date");
            std::vector<QVariantList*> insertParams;
            QVariantList codeList;
            QVariantList nameList;
            QVariantList dateList;
            QDate date = QDate::currentDate();
            for(VWave& temp : *tempVect) {
                codeList << temp.ts_code;
                nameList << temp.ts_name;
                dateList << date;
            }
            insertParams.push_back(&codeList);
            insertParams.push_back(&nameList);
            insertParams.push_back(&dateList);
            instance.executeInsert("v_wave", columns, insertParams, defaultDatabase);
        }
    }

    boost::chrono::duration<double> end_time = boost::chrono::system_clock::now() - now;
    std::cout << "all finished in " << end_time.count() << "seconds" << std::endl;
}

/**
 * 寻找价格处于历史低点的股票
 */
void Calculator::findInLowPrice() noexcept {
    struct in_low {
        QString ts_code, ts_name;
        float curr_price;
    };

    DataCenter& instance = DataCenter::getInstance();
    // 计算之前首先先把要生成天的数据删掉
    QDate date = QDate::currentDate();
    QString dateStr = date.toString("yyyy-MM-dd");
    QString delWhere(" date='");
    delWhere.append(dateStr).append("'");
    instance.executeDel("in_low", delWhere, defaultDatabase);

    std::vector<in_low> rst;
    for(size_t i = 0;i < stockList.size();i++) {
        in_low temp;
        QString ts_code = stockList[i].ts_code;
        StockBatchInfo dayInfo = instance.getStockDayInfo(ts_code.toStdString(), defaultDatabase);
        int lastIndex = dayInfo.info_list.size() - 1;
        if(lastIndex < 0) {
            continue;
        }
        // 查询N天之内的最低价格，现在默认N为90天吧
        if(dayInfo.info_list.size() < 90) {
            continue;
        }

        // 第一步：查找最近N天之内的最低价，收盘价
        float min_price = 100000000;
        for(int i = dayInfo.info_list.size() - 90;i < dayInfo.info_list.size();i++) {
            StockBatchInfo::SingleInfo& tempInfo = dayInfo.info_list[i];
            min_price = min_price < tempInfo.close ? min_price : tempInfo.close;
        }
        // 第二步:最后一天的收盘价
        float last_close = dayInfo.info_list[dayInfo.info_list.size() - 1].close;
        // 第三步：查看一下最后价格同最低价之间的百分比，当前价格比最低价的涨幅低于10%，那么可以放入到数据库in_low当中
        float pct = (last_close - min_price) / min_price;
        if(pct >= 0.1f) {
            continue;
        }

        in_low value;
        value.ts_code = ts_code;
        value.ts_name = dayInfo.ts_name;
        value.curr_price = last_close;
        rst.push_back(value);
    }

    if(rst.size() > 0) {
        std::vector<std::string> columns;
        columns.push_back("ts_code");
        columns.push_back("ts_name");
        columns.push_back("date");
        std::vector<QVariantList*> insertParams;
        QVariantList codeList;
        QVariantList nameList;
        QVariantList dateList;
        QVariantList upPctList;
        QDate date = QDate::currentDate();
        for(in_low& temp : rst) {
            codeList << temp.ts_code;
            nameList << temp.ts_name;
            dateList << date;
        }
        insertParams.push_back(&codeList);
        insertParams.push_back(&nameList);
        insertParams.push_back(&dateList);
        instance.executeInsert("quick_up", columns, insertParams, defaultDatabase);
    }
}

/**
 * 查询昨天涨停的，目前规则比较简单：
 * 涨幅大于9%;
 */
void Calculator::lastDayMaxUp() noexcept {
    struct LastMaxUp {
        QString ts_code, ts_name;
    };

    DataCenter& instance = DataCenter::getInstance();
    // 计算之前首先先把要生成天的数据删掉
    QDate date = QDate::currentDate();
    QString dateStr = date.toString("yyyy-MM-dd");
    QString delWhere(" date='");
    delWhere.append(dateStr).append("'");
    instance.executeDel("last_max_up", delWhere, defaultDatabase);

    std::vector<LastMaxUp> rst;

    for(size_t i = 0;i < stockList.size();i++) {
        LastMaxUp temp;
        QString ts_code = stockList[i].ts_code;
        StockBatchInfo dayInfo = instance.getStockDayInfo(ts_code.toStdString(), defaultDatabase);
        int lastIndex = dayInfo.info_list.size() - 1;
        if(lastIndex < 0) {
            continue;
        }
        StockBatchInfo::SingleInfo& lastInfo = dayInfo.info_list[lastIndex];
        if(lastInfo.pct_chg > 9) {
            temp.ts_code = ts_code;
            temp.ts_name = dayInfo.ts_name;
            rst.push_back(temp);
        }
    }

    if(rst.size() > 0) {
        std::vector<std::string> columns;
        columns.push_back("ts_code");
        columns.push_back("ts_name");
        columns.push_back("date");
        std::vector<QVariantList*> insertParams;
        QVariantList codeList;
        QVariantList nameList;
        QVariantList dateList;
        QDate date = QDate::currentDate();
        for(LastMaxUp& temp : rst) {
            codeList << temp.ts_code;
            nameList << temp.ts_name;
            dateList << date;
        }
        insertParams.push_back(&codeList);
        insertParams.push_back(&nameList);
        insertParams.push_back(&dateList);
        instance.executeInsert("last_max_up", columns, insertParams, defaultDatabase);
    }
}

void Calculator::findContinueUpStock(int up_days) noexcept {

    struct MaxUpRst {
        QString ts_code, ts_name;
        double up_pct;
    };

    DataCenter& instance = DataCenter::getInstance();
    std::string spe_fileter(" order by trade_date desc limit 40");

    // 计算之前首先先把要生成天的数据删掉
    QDate date = QDate::currentDate();
    QString dateStr = date.toString("yyyy-MM-dd");
    QString delWhere(" date='");
    delWhere.append(dateStr).append("'");
    instance.executeDel("quick_up", delWhere, defaultDatabase);

    // higher_c_o: 最后一天收盘价比前一天的收盘价或者开盘价当中较高的更高的结果
    // higher_max: 最后一天收盘价比前一天的最高价更高的结果集
    // common_rst: 除上述两条之外筛选出来的结果
    std::vector<MaxUpRst> higher_c_o, higher_max, common_rst;

    for(size_t i = 0;i < stockList.size();i++) {
        QString ts_code = stockList[i].ts_code;
        StockBatchInfo dayInfo = instance.getStockDayInfo(ts_code.toStdString(), defaultDatabase, spe_fileter);
        int lastIndex = dayInfo.info_list.size() - 1;
        double up_pct = 1.0;
        bool continue_up = dayInfo.info_list.size() > up_days;
        StockBatchInfo::SingleInfo currDaySingleInfo;
        if(dayInfo.info_list.size() > up_days) {
            for(int i = 1;i < up_days;i++) {
                currDaySingleInfo = dayInfo.info_list[i];
                if(currDaySingleInfo.close > currDaySingleInfo.open && continue_up) {
                    continue_up = true;
                    up_pct = up_pct * (1 + static_cast<double>(currDaySingleInfo.pct_chg) / 100);
                }
                else {
                    continue_up = false;
                }
            }
        }

        if(continue_up && currDaySingleInfo.close > currDaySingleInfo.open) {
            currDaySingleInfo = dayInfo.info_list[lastIndex];
            StockBatchInfo::SingleInfo preDaySingleInfo = dayInfo.info_list[lastIndex - 1];
            up_pct = up_pct * (1 + static_cast<double>(currDaySingleInfo.pct_chg) / 100);

            float pre_open = preDaySingleInfo.open;
            float pre_close = preDaySingleInfo.close;
            float o_c_max = pre_open > pre_close ? pre_open : pre_close;
            float pre_max = preDaySingleInfo.high;
            MaxUpRst tempRst;
            tempRst.ts_code = dayInfo.ts_code;
            tempRst.ts_name = dayInfo.ts_name;
            tempRst.up_pct = up_pct;
            if(currDaySingleInfo.close > o_c_max)
                higher_c_o.push_back(tempRst);
            else if(currDaySingleInfo.close > pre_max)
                higher_max.push_back(tempRst);
            else
                common_rst.push_back(tempRst);
        }

    }

    std::vector<MaxUpRst>* infoVectorArr[] = {&higher_c_o, &higher_max, &common_rst};
    for(std::vector<MaxUpRst>* tempVector : infoVectorArr) {
        if(tempVector->size() > 0) {
            std::vector<std::string> columns;
            columns.push_back("ts_code");
            columns.push_back("ts_name");
            columns.push_back("date");
            columns.push_back("up_pct");
            std::vector<QVariantList*> insertParams;
            QVariantList codeList;
            QVariantList nameList;
            QVariantList dateList;
            QVariantList upPctList;
            QDate date = QDate::currentDate();
            for(MaxUpRst& temp : *tempVector) {
                codeList << temp.ts_code;
                nameList << temp.ts_name;
                dateList << date;
                upPctList << temp.up_pct;
            }
            insertParams.push_back(&codeList);
            insertParams.push_back(&nameList);
            insertParams.push_back(&dateList);
            insertParams.push_back(&upPctList);
            instance.executeInsert("quick_up", columns, insertParams, defaultDatabase);
        }
    }
}

void Calculator::anaIndexInfo() noexcept {
    DataCenter& instance = DataCenter::getInstance();

}

void Calculator::startCalcualte() noexcept {
    Calculator calculator;
    boost::thread calThread(calculator);
    calThread.detach();
}

void Calculator::findBigWave(int calDays) noexcept {
    struct BigWave {
        QString ts_code, ts_name;
        double stddev, ave;
    };

    std::vector<BigWave> rst;
    DataCenter& instance = DataCenter::getInstance();

    // 计算之前首先先把要生成天的数据删掉
    QDate date = QDate::currentDate();
    QString dateStr = date.toString("yyyy-MM-dd");
    QString delWhere(" date='");
    delWhere.append(dateStr).append("'");
    instance.executeDel("big_wave", delWhere, defaultDatabase);

    for(size_t i = 0;i < stockList.size();i++) {
        BigWave tempRst;
        QString ts_code = stockList[i].ts_code;
        StockBatchInfo dayInfo = instance.getStockDayInfo(ts_code.toStdString(), defaultDatabase);

        tempRst.ts_code = ts_code;
        tempRst.ts_name = dayInfo.ts_name;
        if(dayInfo.info_list.size() > calDays) {
            auto rbegin = dayInfo.info_list.rbegin(),
                    rend = dayInfo.info_list.rbegin() + calDays;
            float total = 0;
            for(;rbegin != rend;rbegin++) {
                total += rbegin->pct_chg;
            }
            tempRst.ave = total / calDays;
            rbegin = dayInfo.info_list.rbegin();
            float aveDelta2Total = 0;
            for(;rbegin != rend;rbegin++) {
                aveDelta2Total += (rbegin->pct_chg - tempRst.ave) *
                        (rbegin->pct_chg - tempRst.ave);
            }
            tempRst.stddev = aveDelta2Total / calDays;
            rst.push_back(tempRst);
        }
    }

    // 按照降序拍一下序
    std::sort(rst.begin(), rst.end(), [](BigWave& one, BigWave& two) -> bool {
        return one.stddev - two.stddev > 0.0001 ? false : true;
    });

    if(rst.size() > 0) {
        std::vector<std::string> columns;
        columns.push_back("ts_code");
        columns.push_back("ts_name");
        columns.push_back("date");
        columns.push_back("stddev");
        columns.push_back("ave");
        std::vector<QVariantList*> insertParams;
        QVariantList codeList;
        QVariantList nameList;
        QVariantList dateList;
        QVariantList stddevList;
        QVariantList aveList;
        QDate date = QDate::currentDate();
        for(BigWave& temp : rst) {
            codeList << temp.ts_code;
            nameList << temp.ts_name;
            dateList << date;
            stddevList << temp.stddev;
            aveList << temp.ave;
        }
        insertParams.push_back(&codeList);
        insertParams.push_back(&nameList);
        insertParams.push_back(&dateList);
        insertParams.push_back(&stddevList);
        insertParams.push_back(&aveList);
        instance.executeInsert("big_wave", columns, insertParams,
                               defaultDatabase);
    }
}

void Calculator::findBigDown() noexcept {
    struct BigDown {
        QString ts_code ,ts_name;
    };

    std::vector<BigDown> rst;
    DataCenter& instance = DataCenter::getInstance();

    //　查询出所有已经添加到数据库当中的记录，如有有重复则不添加
    std::vector<QString> alreadyContain;
    QString sql("select ts_code from big_down where del_date is null");
    instance.executeQuery(sql, [&alreadyContain](QSqlQuery& query) -> void {
        while(query.next()) {
            alreadyContain.push_back(query.value("ts_code").toString());
        }
    }, defaultDatabase);

    // 跌幅达到了３５％添加到列表当中
    for(size_t i = 0;i < stockList.size();i++) {
        BigDown tempRst;
        QString ts_code = stockList[i].ts_code;
        StockBatchInfo dayInfo = instance.getStockDayInfo(ts_code.toStdString(), defaultDatabase);
        if(dayInfo.info_list.size() == 0) {
            continue;
        }

        float currPrice = dayInfo.info_list[dayInfo.info_list.size() - 1].close;
        for(int j = dayInfo.info_list.size() - 1;j >= 0;j--) {
            float tempPrice = dayInfo.info_list[j].close;
            float changePct = (currPrice - tempPrice) / tempPrice;
            // 实际上这只股票一直处于上涨过程当中，并且涨幅达到了１５％，放弃，看下一个
            if(changePct > 0.15f) {
                break;
            }

            // 如果一直下跌，并且跌幅达到了３５％并且没有在已添加列表当中，加入到候选列表当中
            if(changePct < -0.35f &&
                    std::find(alreadyContain.begin(),
                              alreadyContain.end(),
                              ts_code) == alreadyContain.end()) {
                tempRst.ts_code = ts_code;
                tempRst.ts_name = dayInfo.ts_name;
                rst.push_back(tempRst);
                break;
            }
        }
    }

    if(rst.size() > 0) {
        std::vector<std::string> columns;
        columns.push_back("ts_code");
        columns.push_back("ts_findBigDownThenUpname");
        columns.push_back("add_date");
        std::vector<QVariantList*> insertParams;
        QVariantList codeList;
        QVariantList nameList;
        QVariantList addDateList;
        QDate date = QDate::currentDate();
        for(BigDown& temp : rst) {
            codeList << temp.ts_code;
            nameList << temp.ts_name;
            addDateList << date;
        }
        insertParams.push_back(&codeList);
        insertParams.push_back(&nameList);
        insertParams.push_back(&addDateList);
        instance.executeInsert("big_down", columns, insertParams,
                               defaultDatabase);
    }
}

std::vector<float> Calculator::calculateMA(StockBatchInfo& info, int begin,
                                           int end, int ma) noexcept {
    if(info.info_list.size() == 0 || info.info_list.size() - 1 < begin
            || info.info_list.size() - 1 < end || ma <= 1) {
        return std::vector<float>();
    }

    float sumVal = 0;
    std::vector<float> rst;
    begin = begin - ma + 1 >= 0 ? begin - ma + 1 : begin;

    for(int i = begin;i < begin + ma && i < info.info_list.size();i++) {
        sumVal += info.info_list[i].close;
    }

    for(int i = begin + ma;i <= end && i < info.info_list.size();i++) {
        rst.push_back(sumVal / ma);
        // 减去第一个值
        sumVal -= info.info_list[i - ma].close;
        // 把当前值加入到总和当中
        sumVal += info.info_list[i].close;
    }
    return rst;
}

void Calculator::findBigDownThenUp() noexcept {

    struct BigDownUp {
        QString ts_code, ts_name;
    };

    std::vector<BigDownUp> rst;
    DataCenter& instance = DataCenter::getInstance();
    // 查询出所有的大跌后的股票
    QString queryDownUp("select ts_code from big_down");

    std::vector<QString> checkRange;
    instance.executeQuery(queryDownUp, [&checkRange](QSqlQuery& query) -> void {
        while(query.next()) {
            checkRange.push_back(query.value("ts_code").toString());
        }
    }, defaultDatabase);

    // 查询已经存在于列表当中的
    std::vector<QString> exits;
    QString queryExists("select ts_code from big_down_up where del_date is null");
    instance.executeQuery(queryExists, [&exits](QSqlQuery& query) -> void {
        while(query.next()) {
            exits.push_back(query.value("ts_code").toString());
        }
    }, defaultDatabase);

    // 开始筛选工作
    for(size_t i = 0;i < checkRange.size();i++) {
        BigDownUp temp;
        QString ts_code = checkRange[i];
        StockBatchInfo info = instance.getStockDayInfo(ts_code.toStdString(), defaultDatabase);

        // 计算一下ＭＡ值，看是否连续两天ＭＡ５开始上涨
        if(info.info_list.size() > 10) {
            int calBegin = info.info_list.size() - 10;
            int calEnd = info.info_list.size() - 1;
            std::vector<float> ma = calculateMA(info, calBegin, calEnd, 5);
            float tempMA = ma.size() > 0 ? ma[ma.size() - 1] : 0;
            for(size_t j = ma.size() - 2; j < ma.size();j--) {
                // 只计算一个就可以了
                if(tempMA > ma[j] &&
                        std::find(exits.begin(),
                                  exits.end(),
                                  ts_code) == exits.end()) {
                    temp.ts_code = info.ts_code;
                    temp.ts_name = info.ts_name;
                    rst.push_back(temp);
                    break;
                }
            }
        }
    }

    if(rst.size() > 0) {
        std::vector<std::string> columns;
        columns.push_back("ts_code");
        columns.push_back("ts_name");
        columns.push_back("add_date");
        std::vector<QVariantList*> insertParams;
        QVariantList codeList;
        QVariantList nameList;
        QVariantList addDateList;
        QDate date = QDate::currentDate();
        for(BigDownUp& temp : rst) {
            codeList << temp.ts_code;
            nameList << temp.ts_name;
            addDateList << date;
        }
        insertParams.push_back(&codeList);
        insertParams.push_back(&nameList);
        insertParams.push_back(&addDateList);
        instance.executeInsert("big_down_up", columns, insertParams,
                               defaultDatabase);
    }
}
