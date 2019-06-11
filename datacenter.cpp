#include "datacenter.h"
#include <QSqlQuery>
#include <QVariant>

DataCenter::DataCenter() {
    defaultDatabase = QSqlDatabase::addDatabase("QMYSQL");
    defaultDatabase.setHostName("localhost");
    defaultDatabase.setDatabaseName("stock");
    defaultDatabase.setUserName("root");
    defaultDatabase.setPassword("123");
    defaultDatabase.open();

    redis = redisConnect("127.0.0.1", 6379);
}

DataCenter::~DataCenter() {
    if(defaultDatabase.isOpen())
        defaultDatabase.close();

    // 释放redis连接
    if(redis != nullptr) {
        redisFree(redis);
    }
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
    vector.push_back("603069.SH");
    dataFetch.fetchIndexData(vector);
}
