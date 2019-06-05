#ifndef DATACENTER_H
#define DATACENTER_H

#include <QObject>
//#include <hiredis.h>
#include <QVector>
#include <QMap>
#include <QSqlDatabase>

#include "stockinfo.h"

class DataCenter: public QObject {
	Q_OBJECT

    enum {
        K_INFO, INDEX_INFO
    };
public:
    DataCenter();

    ~DataCenter();

    StockBatchInfo* getStockBatchInfoByTsCode(QString ts_code);

    void executeQuery(QString querySql, std::function<void (QSqlQuery&)>);
signals:
	void indexInfoChanged();
private:

    QMap<QString, StockBatchInfo> kInfoMap;
    QSqlDatabase defaultDatabase;
};

#endif // DATACENTER_H
