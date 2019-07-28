#include <vector>
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

void Calculator::operator()() noexcept {
    findVWaveStock(3);
    findContinueUpStock(3);
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

    std::vector<StockBaseInfo> stockList = instance.getStockList(defaultDatabase);
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

void Calculator::findContinueUpStock(int up_days) noexcept {

    struct MaxUpRst {
        QString ts_code, ts_name;
        double up_pct;
    };

    DataCenter& instance = DataCenter::getInstance();
    std::vector<StockBaseInfo> stockList = instance.getStockList(defaultDatabase);
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
