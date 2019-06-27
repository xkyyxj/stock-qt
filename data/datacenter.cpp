// 奇怪的问题：hiRedis.h如果在Boost.Beast之前import的话，会导致编译报错，之后就没有问题了
// 定义所有的Boost库都采用动态链接的方式进行链接
#define BOOST_ALL_DYN_LINK
#include <QSqlQuery>
#include <QVariant>
#include <QTextCodec>
#include "datacenter.h"
#include "stockindexfetch.h"
#include <boost/thread.hpp>
#include <iostream>
#include <boost/algorithm/string.hpp>

DataCenter* DataCenter::dataCenter = new DataCenter();

DataCenter& DataCenter::getInstance() {
    return *dataCenter;
}

DataCenter::DataCenter() {
    defaultDatabase = QSqlDatabase::addDatabase("QMYSQL");
    defaultDatabase.setHostName("localhost");
    defaultDatabase.setDatabaseName("stock");
    defaultDatabase.setUserName("root");
    defaultDatabase.setPassword("123");
    defaultDatabase.open();

    compressMap[boost::this_thread::get_id()] = zlib::ZLibCompress();
}

DataCenter::~DataCenter() {
    if(defaultDatabase.isOpen())
        defaultDatabase.close();
}

StockBatchInfo* DataCenter::getStockBatchInfoByTsCode(QString ts_code) {
    // 首先查看一下是否存在于map当中
    if(kInfoMap.contains(ts_code)) {
        return &kInfoMap[ts_code];
    }

    // 否则的话，从Redis缓存当中获取


    // 再否则的话，从MySQL数据库当中获取
    StockBatchInfo batchInfo;
    QSqlQuery query;
    QString querySql("select * from stock_base_info where ts_code=?");
    query.prepare(querySql);
    query.addBindValue(QVariant(ts_code));
    query.exec();
    batchInfo.addSingleDayInfos(query);
    batchInfo.setTsCode(std::move(ts_code));

    // 查询一下ts_name
    querySql = "select ts_name from stock_list where ts_code=?";
    query.prepare(querySql);
    query.addBindValue(QVariant(ts_code));
    while(query.next()) {
        batchInfo.setTsName(query.value("ts_name").toString());
    }
    kInfoMap.insert(ts_code, batchInfo);

    return &kInfoMap[ts_code];
}

void DataCenter::executeQuery(QString querySql, std::function<void (QSqlQuery&)> callback) {
    QSqlQuery query;
    query.prepare(querySql);
    query.exec();
    callback(query);
}

std::string DataCenter::getStockIndexInfoStr(std::string& code) {
    auto iteBegin = containsMap.begin();
    auto iteEnd = containsMap.end();
    boost::thread::id lockId;
    bool found = false;
    for(;iteBegin != iteEnd;iteBegin++) {
        if(iteBegin->second.contains(QString::fromStdString(code))) {
            lockId = iteBegin->first;
            found = true;
            break;
        }
    }

    if(!found) {
        return std::string();
    }

    boost::lock_guard<boost::mutex> guard(*mutextMap[lockId]);
    std::string finalRst;
    std::string currIndexInfo = idToConMapMap[lockId][code];
    std::string finalIndexInfo;
    redisCache.getBinaryDataFromRedis(code, [this, &finalIndexInfo, &currIndexInfo](char* rst, size_t size) -> void {
        if(rst != nullptr) {
            if(rst != nullptr) {
                // 解压一下字符串
                boost::thread::id currId = boost::this_thread::get_id();
                compressMap[currId].startDecompress();
                std::vector<unsigned char> realRst = compressMap[currId].endDecompress(reinterpret_cast<unsigned char*>(rst), size);

                char* tempCharArray = reinterpret_cast<char*>(realRst.data());
                std::string originStr(tempCharArray);
                StockIndexBatchInfo::mergeTwoEncodeStr(originStr, currIndexInfo);
                finalIndexInfo = std::move(originStr);
            }
            else {
                finalIndexInfo = std::move(currIndexInfo);
            }
        }
    });
    return finalIndexInfo;
}

StockIndexBatchInfo DataCenter::getStockIndexInfo(std::string& code) {
    std::string infoStr = getStockIndexInfoStr(code);
    StockIndexBatchInfo info;
    if(infoStr.size() > 0) {
        info.decodeFromStr(infoStr);
    }
    return info;
}

void DataCenter::writeIndexInfo(std::string& input, bool syncToRedis) {
    DataCenter& instance = DataCenter::getInstance();
    boost::thread::id currId = boost::this_thread::get_id();

    // 将输入字符串分割
    std::vector<std::string> v;
    // 从Sina当中获取到的字符串当中包含了\n，因此过滤掉，利用boost::split的token_compress_on
    boost::split(v, input, boost::is_any_of(";\n"), boost::token_compress_on);
    // 加锁
    boost::lock_guard<boost::mutex> guard(*instance.mutextMap[currId]);

    // 获取已有字符串
    std::string realCode;
    std::string currIndexInfo;
    for(auto temp : v) {
        // boost::split分割的时候，最后的一个字符是\n，会留下一个空串
        if(temp.size() == 0)
            return;
        realCode = temp.substr(11, 8);

        // 将sina当中获取的格式转换为目标格式：000001.SZ
        if(realCode.find("sz") == std::string::npos) {
            realCode = std::string(realCode.substr(2, 6));
            realCode.append(".SZ");
        }
        else {
            realCode = std::string(realCode.substr(2, 6));
            realCode.append(".SH");
        }

        if(instance.idToConMapMap[currId].find(realCode) != instance.idToConMapMap[currId].end()) {
            std::string& origin = instance.idToConMapMap[currId].at(realCode);
            currIndexInfo = StockIndexBatchInfo::appendEncodeUseSina(origin, temp);
        }
        else {
            std::string emptyStr;
            currIndexInfo = StockIndexBatchInfo::appendEncodeUseSina(emptyStr, temp);
        }

        if(syncToRedis) {
            // 将原先Redis缓存当中存储的取出来，然后与当前程序缓存的拼接起来，一起写入到Redis当中
            std::string finalIndexInfo;
            instance.redisMap[currId].getBinaryDataFromRedis(realCode, [&instance, &currId, &currIndexInfo, &finalIndexInfo](char* rst, size_t size) -> void {
                if(rst != nullptr) {
                    // 解压一下字符串
                    instance.compressMap[currId].startDecompress();
                    std::vector<unsigned char> realRst = instance.compressMap[currId].endDecompress(reinterpret_cast<unsigned char*>(rst), size);

                    char* tempCharArray = reinterpret_cast<char*>(realRst.data());
                    std::string originStr(tempCharArray);
                    StockIndexBatchInfo::mergeTwoEncodeStr(originStr, currIndexInfo);
                    finalIndexInfo = std::move(originStr);
                }
                else {
                    finalIndexInfo = std::move(currIndexInfo);
                }
            });

            // 写入Redis缓存
            instance.compressMap[currId].startCompress();
            std::vector<unsigned char> compressRst = instance.compressMap[currId].endCompress(finalIndexInfo);
            instance.redisMap[currId].writeBinaryDataToStr(realCode, compressRst.data(), compressRst.size());
        }

        instance.idToConMapMap[currId][realCode] = !syncToRedis ? std::move(currIndexInfo) : std::string();
    }
}

void DataCenter::startFetchIndexInfo() {
    QVector<QString> vector;

    QString stockListQrySql("select ts_code from stock_list where market in ('主板', '中小板')");

    executeQuery(stockListQrySql, [&vector, this](QSqlQuery& qryRst) -> void {
        while(qryRst.next()) {
            vector.push_back(qryRst.value("ts_code").toString());
        }
    });

    // 将拉取分时数据的任务分配为多个线程
    // 每个线程分配330个股票拉取，计算一下线程的数量
    unsigned long const hardware_threads = boost::thread::hardware_concurrency();
    unsigned long const num_threads = hardware_threads != 0 ? hardware_threads : 2;
    unsigned long need_threads = vector.size() / 330;
    need_threads += vector.size() % 330 > 0 ? 1 : 0;
    need_threads = num_threads > need_threads ? need_threads : num_threads;

    std::cout << "my thread number is " << need_threads << std::endl;
    int each_threads_pro_num = static_cast<int>(vector.size() / need_threads);
    int startIndex = 0;
    for(int i = 0;i < need_threads;i++) {
        QVector<QString> tempVector;
        int count = 0;
        for(;startIndex < vector.size() && count < each_threads_pro_num;startIndex++, count++) {
            tempVector.push_back(vector[startIndex]);
        }
        if(tempVector.size() > 0) {
            boost::function<void (std::string&, bool)> tempFun;
            tempFun = &DataCenter::writeIndexInfo;
            StockIndexFetch tempDataFetch(tempVector, tempFun);
            boost::thread tempThread(tempDataFetch);

            // 初始化有关于实时信息的线程安全措施以及内容存储容器
            containsMap[tempThread.get_id()] = std::move(tempVector);
            mutextMap[tempThread.get_id()] = new boost::mutex();
            idToConMapMap[tempThread.get_id()] = std::map<std::string, std::string>();

            compressMap[tempThread.get_id()] = zlib::ZLibCompress();
            redisMap[tempThread.get_id()] = RedisCacheTools();

            tempThread.detach();    //分离线程，后台运行
        }
    }


    //dataFetch.fetchIndexData(vector);
}
