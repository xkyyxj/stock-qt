#include "commonanaresult.h"

#include "data/datacenter.h"

CommonAnaResult::CommonAnaResult(QString _tableMeta): AnaResult (_tableMeta)
{
    isValid = false;
}


void CommonAnaResult::initDataFromDB() {
    bool is_redis;
    DataCenter& instance = DataCenter::getInstance();
    QString queryTableName("select table_name, is_redis from table_meta where pk_tablemeta='");
    queryTableName.append(tableMeta).append("'");
    instance.executeQuery(queryTableName, [this, &is_redis](QSqlQuery& query) -> void {
        while(query.next()) {
            isValid = true;
            dbTableName = query.value("table_name").toString();
            QString is_redis_str = query.value("is_redis").toString();
            is_redis = query.value("is_redis").toString() == "Y";
        }
    }, QSqlDatabase());

    if(!isValid) {
        return;
    }

    if(!is_redis) {
        QString tableDetailInfoQry("select * from table_column where pk_tablemeta='");
        tableDetailInfoQry.append(tableMeta).append("'");
        instance.executeQuery(tableDetailInfoQry, [this](QSqlQuery& query) -> void {
            while(query.next()) {
                dbColumns.push_back(query.value("column_name").toString());
                displayHead.push_back(query.value("display_name").toString());
            }
        }, QSqlDatabase());

        QDate currDate = QDate::currentDate();
        QString dateStr = currDate.toString("yyyy-MM-dd");
        QString filter(" where date='");
        filter.append(dateStr).append("'");

        // 通知表格model进行数据更新
        if(dbTableName.size() > 0) {
            // 拼接SQL片段
            QString querySql = "select ";
            QString tempColumns;
            for(size_t i = 0;i < dbColumns.size() - 1;i++) {
                querySql.append(dbColumns[i]).append(",");
            }
            querySql.append(dbColumns[dbColumns.size() - 1]);
            querySql.append(" from ").append(dbTableName).append(" ");
            querySql.append(filter);

            // 借用回调函数，由DataCenter执行查询
            instance.executeQuery(querySql, [this, &tempColumns](QSqlQuery& query) -> void {
                data.clear();
                while(query.next()) {
                    std::vector<QVariant> tempRow;
                    foreach(tempColumns, dbColumns) {
                        tempRow.push_back(query.value(tempColumns));
                    }
                    data.push_back(std::move(tempRow));
                }
            }, QSqlDatabase());
        }
    }
}
