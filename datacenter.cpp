// 奇怪的问题：hiRedis.h如果在Boost.Beast之前import的话，会导致编译报错，之后就没有问题了
// 定义所有的Boost库都采用动态链接的方式进行链接
#define BOOST_ALL_DYN_LINK
#include <QSqlQuery>
#include <QVariant>
#include <QTextCodec>
#include "datacenter.h"
#include "stockindexfetch.h"
#include <boost/thread.hpp>

DataCenter::DataCenter() {
    defaultDatabase = QSqlDatabase::addDatabase("QMYSQL");
    defaultDatabase.setHostName("localhost");
    defaultDatabase.setDatabaseName("stock");
    defaultDatabase.setUserName("root");
    defaultDatabase.setPassword("123");
    defaultDatabase.open();
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

    int each_threads_pro_num = static_cast<int>(vector.size() / need_threads);
    int startIndex = 0;
    for(int i = 0;i < need_threads;i++) {
        QVector<QString> tempVector;
        int count = 0;
        for(;startIndex < vector.size() && count < each_threads_pro_num;startIndex++, count++) {
            tempVector.push_back(vector[startIndex]);
        }
        if(tempVector.size() > 0) {
            StockIndexFetch tempDataFetch(std::move(tempVector));
            boost::thread tempThread(tempDataFetch);
            tempThread.detach();    //分离线程，后台运行
        }
    }


    //dataFetch.fetchIndexData(vector);
}
