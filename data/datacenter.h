#ifndef DATACENTER_H
#define DATACENTER_H

#include <QObject>
#include <QVector>
#include <QMap>
#pragma execution_character_set("utf-8")
#include <QSqlDatabase>
#include "utils/rediscachetools.h"
#include "stockinfo.h"
#include "datafetch.h"
#include <string>
#include <vector>
#include <map>
#include <boost/thread.hpp>
#include "utils/rediscachetools.h"
#include "utils/zlibcompress.h"

class StockBaseInfo;

class DataCenter: public QObject {
	Q_OBJECT

    enum {
        K_INFO, INDEX_INFO
    };

    DataCenter();

public:
    static DataCenter& getInstance();

    ~DataCenter();

    StockBatchInfo* getStockBatchInfoByTsCode(QString ts_code);

    void executeQuery(QString querySql, std::function<void (QSqlQuery&)>);

    void executeInsert(std::string, std::vector<std::string>&, std::vector<QVariantList*>);

    void startFetchIndexInfo(); // 开始获取股票的每日信息

    std::vector<StockBaseInfo> getStockList() noexcept;

    StockBatchInfo getStockDayInfo(const std::string& ts_code) noexcept;

    static void writeIndexInfo(std::string&, bool syncToRedis);

    StockIndexBatchInfo getStockIndexInfo(std::string& code);

    std::string getStockIndexInfoStr(std::string&);
signals:
	void indexInfoChanged();
private:

    std::map<QString, StockBatchInfo> kInfoMap;
    QSqlDatabase defaultDatabase;

    RedisCacheTools redisCache;

    std::vector<std::map<std::string, std::string>> indexInfoMap;

    static DataCenter* dataCenter;

    // 实时信息相关存储变量
    // 对每个线程要访问的map加锁
    std::map<boost::thread::id, boost::mutex*> mutextMap;
    // 每个线程获取的股票编码列表
    std::map<boost::thread::id, QVector<QString>> containsMap;
    // 每个线程写入的map映射
    std::map<boost::thread::id, std::map<std::string, std::string>> idToConMapMap;

    // 每个线程都有自己的压缩工具（非线程安全）
    std::map<boost::thread::id, zlib::ZLibCompress> compressMap;

    // 每个线程都有自己的Redis链接(非线程安全)
    std::map<boost::thread::id, RedisCacheTools> redisMap;

};

#endif // DATACENTER_H
