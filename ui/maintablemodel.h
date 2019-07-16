#pragma execution_character_set("utf-8")
#ifndef MAINTABLE_H
#define MAINTABLE_H

#include <QAbstractTableModel>
#include "data/datacenter.h"
#include "commonenum.h"

class MainTableModel: public QAbstractTableModel {
public:
    MainTableModel(DataCenter* _dataCenter, QObject* parent = nullptr);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void setFilter(QString _filter);
    void setTableName(QString _tableName);
    void setSelectColumns(std::vector<QString> selectColumns);

    void selectData();

    void setDisplayHeadInfo(std::vector<QString>&);

    void setPrimaryKey(QString& key);

    // 根据QModelIndex返回当前行的主键，主要是为了响应双击事件
    QVariant getPrimaryKeyValue(const QModelIndex& index) const;

public slots:
    void changeTableData(QString& pk);


private:
    QVector<QVector<QVariant>> tableData;
    std::vector<QString> selectColumns;
    QString tableName;
    QString filter;
    DataCenter* dataCenter;
    std::vector<QString> displayHead;

    // 指定当前的主键是什么
    QString primaryKey;
};

#endif // MAINTABLE_H
