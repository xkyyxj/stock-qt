#include "dailyconcern.h"
#include "data/datacenter.h"

DailyConcern::DailyConcern(QString _tableMeta) : AnaResult(_tableMeta)
{
    isValid = false;
}


void DailyConcern::initDataFromDB()
{
    DataCenter& instance = DataCenter::getInstance();

    dbTableName = "daily_concern";

    dbColumns.push_back("ts_code");
    dbColumns.push_back("ts_name");
    dbColumns.push_back("in_datetime");
    dbColumns.push_back("in_price");
    dbColumns.push_back("curr_price");
    dbColumns.push_back("win_pct");

    displayHead.push_back("编码");
    displayHead.push_back("名称");
    displayHead.push_back("收入日期");
    displayHead.push_back("收入价格");
    displayHead.push_back("当前价格");
    displayHead.push_back("盈利百分比");

    // 通知表格model进行数据更新
    if(dbTableName.size() > 0) {
        // 查询出最新的日期
        QString curr_date_sql = "select trade_date from stock_base_info order by trade_date desc limit 1";
        QString last_date_str;
        instance.executeQuery(curr_date_sql, [&last_date_str](QSqlQuery& query) -> void {
            while(query.next()) {
                last_date_str = query.value("trade_date").toString();
            }
        }, QSqlDatabase());
        // 查询出最新的价格
        QString currPriceSql = "select ts_code, close from stock_base_info where ts_code in (select ts_code from daily_concern) and trade_date = '";
        currPriceSql.push_back(last_date_str);
        currPriceSql.push_back("'");
        std::map<QString, double> last_price_map;
        instance.executeQuery(currPriceSql, [&last_price_map](QSqlQuery& query) -> void {
            QString temp_ts_code;
            double close_price;
            while(query.next()) {
                temp_ts_code = query.value("ts_code").toString();
                close_price = query.value("close").toDouble();
                last_price_map[temp_ts_code] = close_price;
            }
        }, QSqlDatabase());

        // 拼接SQL片段
        QString querySql = "select *";
        querySql.append(" from ").append(dbTableName).append(" ");

        // 借用回调函数，由DataCenter执行查询
        QString tempColumns;
        instance.executeQuery(querySql, [this, &last_price_map](QSqlQuery& query) -> void {
            data.clear();
            while(query.next()) {
                std::vector<QVariant> tempRow;
                for(int i = 0;i < 4;i++) {
                    tempRow.push_back(query.value(dbColumns[i]));
                }
                QString ts_code = tempRow[0].toString();
                double in_price = tempRow[3].toDouble();
                double new_close_price = last_price_map[ts_code];
                double up_pct = (new_close_price - in_price) / in_price * 100;
                tempRow.push_back(new_close_price);
                tempRow.push_back(up_pct);
                data.push_back(std::move(tempRow));
            }
        }, QSqlDatabase());
    }
}
