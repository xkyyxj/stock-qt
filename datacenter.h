#ifndef DATACENTER_H
#define DATACENTER_H

#include <QObject>
#include <QVector>
#include <QMap>
#pragma execution_character_set("utf-8")
#include <QSqlDatabase>
#include "rediscachetools.h"
#include "stockinfo.h"
#include "datafetch.h"
#include <string>
#include <vector>
#include <map>

class DataCenter: public QObject {
	Q_OBJECT

    enum {
        K_INFO, INDEX_INFO
    };

public:
    DataCenter();

    static DataCenter& getInstance();

    ~DataCenter();

    StockBatchInfo* getStockBatchInfoByTsCode(QString ts_code);

    void executeQuery(QString querySql, std::function<void (QSqlQuery&)>);

    void startFetchIndexInfo(); // 开始获取股票的每日信息
signals:
	void indexInfoChanged();
private:

    QMap<QString, StockBatchInfo> kInfoMap;
    QSqlDatabase defaultDatabase;

    RedisCacheTools redisCache;

    std::vector<std::map<std::string, std::string>> indexInfoMap;

    static DataCenter* dataCenter;
    static DataCenter& temp;
};

#endif // DATACENTER_H
