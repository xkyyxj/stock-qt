#include "rediscachetools.h"
#include <zlib.h>
#include <hiredis/hiredis.h>
#include <iostream>

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
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "SET %s %b", key.c_str(), data, size));
    if(reply == nullptr) {
        // 失败
        redisFree(redis);
        redis = redisConnect("127.0.0.1", 6379);
        return false;
    }
    else {
        // 处理一下成功的情况
        freeReplyObject(reply);
        return true;
    }
}

// 从Redis当中读取string类型
bool RedisCacheTools::getBinaryDataFromRedis(std::string key, std::function<void(char*, size_t)> callback) noexcept {
    if(!successConnect) {
        return false;
    }

    // 开始正常的读取流程
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis, "GET %s", key.c_str()));
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
        freeReplyObject(reply);
        return true;
    }
}

inline bool RedisCacheTools::isRedisCanUse() noexcept {
    return successConnect;
}

bool RedisCacheTools::pushBackToList(std::string key, std::string value) noexcept {
    if(!successConnect) {
        return false;
    }

    redisReply* reply = static_cast<redisReply*>(redisCommand(redis,
                                                              "rpush %s %s",
                                                              key.c_str(),
                                                              value.c_str()));
    if(reply == nullptr) {
        // 失败
        redisFree(redis);
        redis = redisConnect("127.0.0.1", 6379);
        return false;
    }
    else {
        // 处理一下成功的情况
        freeReplyObject(reply);
        return true;
    }
}

bool RedisCacheTools::prePushToList(std::string key, std::string value) noexcept {
    if(!successConnect) {
        return false;
    }

    // 开始正常的读取流程
    redisReply* reply = static_cast<redisReply*>(redisCommand(redis,
                                                              "lpush %s %s",
                                                              key.c_str(),
                                                              value.c_str()));
    if(reply == nullptr) {
        // 失败
        redisFree(redis);
        redis = redisConnect("127.0.0.1", 6379);
        return false;
    }
    else {
        // 处理一下成功的情况
        freeReplyObject(reply);
        return true;
    }
}

bool RedisCacheTools::pushBackToList(std::string key,
                                     std::vector<std::string>& values) noexcept {
    if(!successConnect) {
        return false;
    }

    if(values.size() == 0) {
        return true;
    }

    // 将操作名称作为argv的第一个
    int size = static_cast<int>(values.size()) + 2;
    const char** argv = new const char*[size];
    argv[0] = "lpush";
    argv[1] = key.c_str();
    for(int i = 2;i < size;i++) {
        argv[i] = values[i - 2].c_str();
    }
    bool retVal = redisCommanWithArgv(size, argv, nullptr);
    delete[] argv;
    return retVal;
}

/**
 * 传入的一切参数都由调用者负责清除
 * 传入的argv格式如下（以lpush key1 val1 val2为例）：
 * argv[0] = "lpush";
 * argv[1] = "key1";
 * argv[2] = "val1";
 * argv[3] = "val2";
 */
bool RedisCacheTools::redisCommanWithArgv(int argc, const char** argv, const size_t*
                         argvlen) noexcept {
    redisReply* reply = static_cast<redisReply*>(
                redisCommandArgv(redis, argc, argv, argvlen));
    if(reply == nullptr) {
        // 失败
        redisFree(redis);
        redis = redisConnect("127.0.0.1", 6379);
        return false;
    }
    else {
        // 处理一下成功的情况
        freeReplyObject(reply);
        return true;
    }
}

/**
 * 传入的一切参数都由调用者负责清除
 * 传入的argv格式如下（以lpush key1 val1 val2为例）：
 * argv[0] = "lpush";
 * argv[1] = "key1";
 * argv[2] = "val1";
 * argv[3] = "val2";
 * 回调函数callback不需要清理redisReply指针，由本函数负责此操作
 */
void RedisCacheTools::redisCommanWithArgvAndCallback(int argc, const char** argv, const size_t*
                                     argvlen, std::function<void (redisReply*)> callback) noexcept {
    redisReply* reply = static_cast<redisReply*>(
                redisCommandArgv(redis, argc, argv, argvlen));
    if(reply == nullptr) {
        // 失败
        redisFree(redis);
        redis = redisConnect("127.0.0.1", 6379);
    }
    else {
        // 处理一下成功的情况
        callback(reply);
        freeReplyObject(reply);
    }
}

/**
 * 删除指定的Key
 */
bool RedisCacheTools::delKey(std::string key) noexcept {
    const char* argv[2];
    argv[0] = "del";
    argv[1] = key.c_str();
    return redisCommanWithArgv(2, argv, nullptr);
}
