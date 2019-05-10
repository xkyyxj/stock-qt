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
    float open, close, high, low;
};

class StockBatchInfo {
public:
    struct SingleInfo {
        QDate tradeDate;
        float open, close, high, low;
    };
    QString ts_code, ts_name;
    QVector<SingleInfo> info_list;

public:
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
