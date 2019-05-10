#ifndef REDISCACHETOOLS_H
#define REDISCACHETOOLS_H

#include <QString>

#include <hiredis.h>
#include "stockinfo.h"

class RedisCacheTools {
public:
    void writeStockInfoToRedis(StockInfo& info);
    void writeStockBatchInfoToRedis(StockBatchInfo& info);

    StockBatchInfo getStockInfoFromRedis(QString stock_code);
};

#endif // REDISCACHETOOLS_H
