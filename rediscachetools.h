#pragma execution_character_set("utf-8")
#ifndef REDISCACHETOOLS_H
#define REDISCACHETOOLS_H

#include <QString>

#include "stockinfo.h"
#include "stockindexinfo.h"

struct redisContext;

class RedisCacheTools {
private:
    redisContext* redis;
    bool successConnect = false;
public:
    RedisCacheTools();
    void writeStockInfoToRedis(StockInfo& info);
    void writeStockBatchInfoToRedis(StockBatchInfo& info);

    void appendStockIndexInfo(StockIndexBatchInfo::SingleIndexInfo&);

    StockBatchInfo getStockInfoFromRedis(QString stock_code);

};

#endif // REDISCACHETOOLS_H
