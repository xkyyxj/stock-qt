﻿#pragma execution_character_set("utf-8")
#ifndef REDISCACHETOOLS_H
#define REDISCACHETOOLS_H

#include <QString>

#include "data/stockinfo.h"
#include "data/stockindexinfo.h"

struct redisContext;
struct redisReply;

class RedisCacheTools {
private:
    redisContext* redis;
    bool successConnect = false;
public:
    RedisCacheTools();
    ~RedisCacheTools();

    RedisCacheTools(const RedisCacheTools&) = delete;

    RedisCacheTools& operator=(const RedisCacheTools&) = delete;

    RedisCacheTools& operator=(RedisCacheTools&&);

    void writeStockInfoToRedis(StockInfo& info);
    void writeStockBatchInfoToRedis(StockBatchInfo& info);

    void appendStockIndexInfo(StockIndexBatchInfo::SingleIndexInfo&);

    StockBatchInfo getStockInfoFromRedis(QString stock_code);

    bool writeStrToRedis(std::string key, std::string val) noexcept;

    bool writeBinaryDataToStr(std::string key, unsigned char* data, size_t size) noexcept;

    bool writeBinaryDataToStr(std::string key, char* data, size_t size) noexcept;

    bool getBinaryDataFromRedis(std::string key, std::function<void(char*, size_t)>) noexcept;

    inline bool isRedisCanUse() noexcept;

    bool pushBackToList(std::string, std::string) noexcept;

    bool prePushToList(std::string, std::string) noexcept;

    bool pushBackToList(std::string, std::vector<std::string>&) noexcept;

    bool redisCommanWithArgv(int argc, const char** argv, const size_t*
                             argvlen) noexcept;
    void redisCommanWithArgvAndCallback(int argc, const char** argv, const size_t*
                             argvlen, std::function<void (redisReply*)>) noexcept;

    bool delKey(std::string key) noexcept;

};

#endif // REDISCACHETOOLS_H
