#pragma execution_character_set("utf-8")
#ifndef STOCKINFO_H
#define STOCKINFO_H

#include <QString>
#include <QDate>
#include <QVector>
#include <QSqlQuery>
#include <QVariant>

class StockInfo {
public:
    QString ts_code, ts_name;
    QDate trade_date;
    float open, close, high, low, pct_chg, pre_close;
};

class StockBatchInfo {
public:
    struct SingleInfo {
        QDate tradeDate;
        float open, close, high, low, pct_chg;
    };
    QString ts_code, ts_name;
    QVector<SingleInfo> info_list;

public:
    StockBatchInfo(StockBatchInfo&& origin) noexcept;
    StockBatchInfo() noexcept {}

    SingleInfo* getOneDayInfo(int i) noexcept;

    StockBatchInfo& operator=(const StockBatchInfo&) = delete;

    int getLength() noexcept;

    StockBatchInfo& operator=(StockBatchInfo&&) noexcept;
    void addSingleDayInfos(QSqlQuery& quryInfo);
    void setTsCode(QString&& code) {
        ts_code = code;
    }

    QString getTsCode() {
        return ts_code;
    }

    void setTsName(QString&& name) {
        ts_name = name;
    }

    QString getTsName() {
        return ts_name;
    }
};

#endif // STOCKINFO_H
