#pragma execution_character_set("utf-8")
#ifndef MAINTABLEMODE_H
#define MAINTABLEMODE_H

#include <QAbstractTableModel>
#include "data/datacenter.h"
#include "commonenum.h"
#include "data/anaresult.h"

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

    inline QString getTableName() noexcept {return tableName;}

    void selectData();

    void setAnaRst(AnaResult* rst) noexcept;

    void setTableData(std::vector<std::vector<QVariant>>&) noexcept;

    void setDisplayHeadInfo(std::vector<QString>&);

    void setPrimaryKey(QString& key);

    // 根据QModelIndex返回当前行的主键，主要QVariant是为了响应双击事件
    QVariant getPrimaryKeyValue(const QModelIndex& index) const;

    void updateView() noexcept;

public slots:
    void changeTableData(QString& pk);


private:
    std::vector<std::vector<QVariant>> tableData;
    std::vector<QString> selectColumns;
    QString tableName;
    QString filter;
    DataCenter* dataCenter;
    std::vector<QString> displayHead;

    // 当前的分析结果实例，包含了解析逻辑以及数据的存储
    AnaResult* anaRst;

    // 指定当前的主键是什么
    QString primaryKey;
};

#endif // MAINTABLEMODE_H
