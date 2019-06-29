#include <vector>
#include <QString>

#include "calculator.h"
#include "data/datacenter.h"
#include "data/stockbaseinfo.h"

/**
 *  查找V型反转，经历了若干天的下跌之后开始第一根阳线
 */
void Calculator::findVWaveStock(int checkDays) noexcept {
    DataCenter& instance = DataCenter::getInstance();

    // 下降过程当中必须是阴线，并且最后一根阳线收盘价高于前一天开盘价
    std::vector<QString> downGreenUpHigh;
    // 下降过程当中必须是阴线，并且最后一根阳线收盘价可低于前一天开盘价
    std::vector<QString> downGreenUpLow;
    // 下降过程当中可以是阳线或阴线，并且最后一根阳线收盘价高于前一天开盘价
    std::vector<QString> downRedUpHigh;
    // 下降过程当中可以是阳线或阴线，并且最后一根阳线收盘价可低于前一天开盘价
    std::vector<QString> downRedUpLow;
    std::vector<QString> finalResult;

    std::vector<StockBaseInfo> stockList = instance.getStockList();
    for(size_t i = 0;i < stockList.size();i++) {
        QString ts_code = stockList[i].ts_code;
        StockBatchInfo dayInfo = instance.getStockDayInfo(ts_code.toStdString());

        bool is_continue_down = false, downGreen = false;
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

        if(is_continue_down) {
            StockBatchInfo::SingleInfo preDayInfo = dayInfo.info_list[lastIndex - 1];
            StockBatchInfo::SingleInfo info = dayInfo.info_list[lastIndex];
            // 最后一天是阳线
            if(info.close > info.open) {
                if(downGreen) {
                    if(info.close - preDayInfo.open > 0.001f) {
                        downGreenUpHigh.push_back(ts_code);
                    }
                    else {
                        downGreenUpLow.push_back(ts_code);
                    }
                }
                else {
                    if(info.close - preDayInfo.open > 0.001f) {
                        downRedUpHigh.push_back(ts_code);
                    }
                    else {
                        downRedUpLow.push_back(ts_code);
                    }
                }
            }
        }
    }

    std::vector<QString>* tempArray[4] = {&downGreenUpHigh, &downGreenUpLow,
                                          &downRedUpLow, &downRedUpHigh};
    for(std::vector<QString>* tempVect : tempArray) {
        if(tempVect->size() > 0) {
            std::vector<std::string> columns(3);
            columns.push_back("ts_code");
            columns.push_back("pk_category");
            columns.push_back("infodate");
            std::vector<QVariantList*> insertParams;
            QVariantList codeList;
            QVariantList categoryList;
            QVariantList dateList;
            QDate date = QDate::currentDate();
            for(QString& temp : *tempVect) {
                codeList << temp;
                categoryList << "1";
                dateList << date;
            }
            insertParams.push_back(&codeList);
            insertParams.push_back(&categoryList);
            insertParams.push_back(&dateList);
            instance.executeInsert("ana_category_detail", columns, insertParams);
        }
    }
}
