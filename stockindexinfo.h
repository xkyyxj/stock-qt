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
    struct SingleIndexInfo {
        QDateTime time;
        float price;
    };
public:
    QString ts_code, ts_name;
    QVector<SingleIndexInfo> info_list;
};

#endif // STOCKINDEXINFO_H
