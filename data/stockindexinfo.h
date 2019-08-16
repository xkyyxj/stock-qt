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

    enum {
        CURR_PRICE, CURR_MAX, CURR_MIN, BUY_ONE_P, SOLD_ONE_P,
        DEAL
    };

    struct SingleIndexInfo {
        QDateTime time;
        double mainContent[27];
    };

    QString ts_code, ts_name;
    float pre_close, today_open;

    std::vector<SingleIndexInfo> info_list;

public:
    void decodeFromStrForSina(QString&);

    void batchDecodeForSina(QString&);

    std::string encodeToStr() noexcept;

    bool decodeFromStr(const std::string&) noexcept;

    std::string& appendEncodeToStr(std::string&) noexcept(false);

    static std::string& appendEncodeUseSina(std::string&, std::string&);

    static void mergeTwoEncodeStr(std::string&, std::string&);
};

#endif // STOCKINDEXINFO_H
