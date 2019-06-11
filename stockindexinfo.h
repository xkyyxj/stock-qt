#ifndef STOCKINDEXINFO_H
#define STOCKINDEXINFO_H

#include <QDateTime>
#include <QString>
#include <QVector>

class StockIndexInfo {
public:
    QString ts_code, ts_name;
    QDateTime currTime;
    float curr_price;
};

class StockIndexBatchInfo {
public:

    struct SingleIndexInfo {
        QDateTime time;
        float price;
    };

    QString ts_code, ts_name;
    QVector<SingleIndexInfo> infoList;

public:
    void decodeFromStr(QString&);
};

#endif // STOCKINDEXINFO_H
