#include "rediscachetools.h"

#include <hiredis.h>

RedisCacheTools::RedisCacheTools() {
    redis = redisConnect("127.0.0.1", 6379);
    if(redis == NULL || redis->err) {
        successConnect = false;
    }
    else {
        successConnect = true;
    }
}

void RedisCacheTools::writeStockInfoToRedis(StockInfo& info) {

}

void RedisCacheTools::writeStockBatchInfoToRedis(StockBatchInfo& info) {

}

StockBatchInfo RedisCacheTools::getStockInfoFromRedis(QString stock_code) {
    return StockBatchInfo();
}

void RedisCacheTools::appendStockIndexInfo(StockIndexBatchInfo::SingleIndexInfo&) {

}
