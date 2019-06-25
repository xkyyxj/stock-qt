#pragma execution_character_set("utf-8")
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
    void decodeFromStrForSina(QString&);

    void batchDecodeForSina(QString&);

    std::string encodeToStr() noexcept;

    bool decodeFromStr(std::string&) noexcept;

    std::string& appendEncodeToStr(std::string&) noexcept(false);

    static std::string& appendEncodeUseSina(std::string&, std::string&);

    static void mergeTwoEncodeStr(std::string&, std::string&);
};

#endif // STOCKINDEXINFO_H
