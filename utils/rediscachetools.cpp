#include "rediscachetools.h"
#include <zlib.h>
#include <hiredis/hiredis.h>

RedisCacheTools::RedisCacheTools() {
    redis = redisConnect("127.0.0.1", 6379);
    if (redis == NULL || redis->err) {
        successConnect = false;
        if (redis) {

            // handle error
        } else {
            //printf("Can't allocate redis context\n");
        }
    }
    else {
        successConnect = true;
    }
}

RedisCacheTools& RedisCacheTools::operator=(RedisCacheTools&& origin) {
    if(this == &origin) {
        return *this;
    }

    redis = origin.redis;
    successConnect = origin.successConnect;
    origin.redis = nullptr;
    origin.successConnect = false;
}

RedisCacheTools::~RedisCacheTools() {
    if(redis != nullptr){
        redisFree(redis);
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

bool RedisCacheTools::writeBinaryDataToStr(std::string key, char* data, size_t size) noexcept {
    return writeBinaryDataToStr(key, reinterpret_cast<unsigned char*>(data), size);
}

bool RedisCacheTools::writeStrToRedis(std::string key, std::string val) noexcept {
    return writeBinaryDataToStr(key, const_cast<char*>(val.data()), val.size());
}

// 往Redis当中写入string类型
bool RedisCacheTools::writeBinaryDataToStr(std::string key, unsigned char* data, size_t size) noexcept {
    if(!successConnect) {
        return false;
    }

    // 开始正常的写入流程
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "SET %s %b", key.data(), data, size));
    if(reply == nullptr) {
        // 失败
        redisFree(redis);
        redis = redisConnect("127.0.0.1", 6379);
        return false;
    }
    else {
        // 处理一下成功的情况
        return true;
    }
}

// 从Redis当中读取string类型
bool RedisCacheTools::getBinaryDataFromRedis(std::string key, std::function<void(char*, size_t)> callback) noexcept {
    if(!successConnect) {
        return false;
    }

    // 开始正常的读取流程
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "GET %s", key.data()));
    if(reply == nullptr) {
        // 失败
        redisFree(redis);
        redis = redisConnect("127.0.0.1", 6379);
        return false;
    }
    else {
        // 处理一下成功的情况
        char* data = reply->str;
        size_t size = reply->len;
        callback(data, size);
        return true;
    }
}

inline bool RedisCacheTools::isRedisCanUse() noexcept {
    return successConnect;
}
