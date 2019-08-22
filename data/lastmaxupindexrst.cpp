#include "lastmaxupindexrst.h"
#include "data/datacenter.h"
#include <hiredis/hiredis.h>

std::string LastMaxUpIndexRst::REDIS_KEYNAME = "lmu_ok";

LastMaxUpIndexRst::LastMaxUpIndexRst(QString _tableMeta): AnaResult (_tableMeta)
{
    dbTableName = QString::fromStdString(REDIS_KEYNAME);
}

void LastMaxUpIndexRst::initDataFromDB() {
    DataCenter& instance = DataCenter::getInstance();
    std::vector<std::string> ts_codes;
    std::vector<double> up_pct;
    const char** argv = new const char*[5];
    argv[0] = "zrevrange";
    argv[1] = REDIS_KEYNAME.c_str();
    argv[2] = "0";
    argv[3] = "-1";
    argv[4] = "withscores";
    instance.redisCommanWithArgvAndCallback(5, argv, nullptr, [&ts_codes, &up_pct](redisReply* reply) -> void {
        // 构造显示数据
        if(reply->type == REDIS_REPLY_ARRAY) {
            size_t size = reply->elements;
            for(size_t i = 0;i < size;i++) {
                if(i % 2 == 0) {
                    ts_codes.push_back(std::string(reply->element[i]->str));
                }
                else {
                    std::string pct(reply->element[i]->str);
                    up_pct.push_back(std::stod(pct));
                }
            }
        }
    });

    delete[] argv;
    if(ts_codes.size() <= 0) {
        return;
    }
    // 查询股票名称等相关信息
    std::string querySql("select name from stock_list where ts_code in ('");
    for(size_t i = 0 ;i < ts_codes.size();i++) {
        querySql.append(ts_codes[i]).append("','");
    }
    querySql.append("') order by find_in_set(ts_code,'");
    for(size_t i = 0 ;i < ts_codes.size();i++) {
        querySql.append(ts_codes[i]).append(",");
    }
    querySql.append("')");

    instance.executeQuery(QString::fromStdString(querySql), [this, &ts_codes, &up_pct](QSqlQuery& query) -> void {
        size_t count = 0;
        while(query.next()) {
            std::vector<QVariant> row;
            row.push_back(QString::fromStdString(ts_codes[count]));
            row.push_back(query.value("name"));
            row.push_back(QString::fromStdString(std::to_string(up_pct[count])));
            data.push_back(row);
            ++count;
        }
    }, QSqlDatabase());

    dbColumns.push_back(QString("ts_code"));
    dbColumns.push_back(QString("ts_name"));
    dbColumns.push_back(QString("up_pct"));

    std::vector<QString> displayNames;
    displayNames.push_back(QString("编码"));
    displayNames.push_back(QString("名称"));
    displayNames.push_back(QString("上涨百分比"));
}
