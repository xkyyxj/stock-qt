#include "maintablemodel.h"
#include <QSqlQuery>

MainTableModel::MainTableModel(DataCenter* _dataCenter, QObject* parent): QAbstractTableModel (parent), tableData () {
    dataCenter = _dataCenter;
}

int MainTableModel::rowCount(const QModelIndex &/*parent*/) const {
    return tableData.size();
}

int MainTableModel::columnCount(const QModelIndex &/*parent*/) const {
    return selectColumns.size();
}

void MainTableModel::setAnaRst(AnaResult* rst) noexcept {
    anaRst = rst;
    tableData = anaRst->getData();
    selectColumns = anaRst->getDBColumns();
    displayHead = anaRst->getDisplayHead();
    endResetModel();
}

QVariant MainTableModel::data(const QModelIndex &index, int role) const {
    if(!index.isValid())
        return QVariant();

    if(role == Qt::TextAlignmentRole) {
        return int(Qt::AlignRight | Qt::AlignVCenter);
    } else if(role == Qt::DisplayRole) {
        return tableData[index.row()][index.column()];
    }
    else if(role == PRIMARY_KEY_ROLE) {
        return getPrimaryKeyValue(index);
    }

    return QVariant();
}

QVariant MainTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    /*if(section < selectColumns.size())
        return selectColumns[section];
    else {
        return QVariant();
    }*/

    if(role == Qt::DisplayRole && orientation == Qt::Horizontal){
        return section < displayHead.size() ? displayHead[section] : QVariant();
    }
    return QAbstractTableModel::headerData(section,orientation,role);
}

void MainTableModel::setFilter(QString _filter) {
    filter = _filter;
}

void MainTableModel::setTableName(QString _tableName) {
    tableName = _tableName;
}

void MainTableModel::setSelectColumns(std::vector<QString> _selectColumns) {
    selectColumns = _selectColumns;
}

void MainTableModel::selectData() {
    // 拼接SQL片段
    QString querySql = "select ";
    QString tempColumns;
    for(size_t i = 0;i < selectColumns.size() - 1;i++) {
        querySql.append(selectColumns[i]).append(",");
    }
    querySql.append(selectColumns[selectColumns.size() - 1]);
    querySql.append(" from ").append(tableName).append(" ");
    querySql.append(filter);

    // 借用回调函数，由DataCenter执行查询
    dataCenter->executeQuery(querySql, [this, &tempColumns](QSqlQuery& query) -> void {
        tableData.clear();
        while(query.next()) {
            std::vector<QVariant> tempRow;
            foreach(tempColumns, selectColumns) {
                tempRow.push_back(query.value(tempColumns));
            }
            tableData.push_back(std::move(tempRow));
        }

        // 通知视图进行更新
        endResetModel();
    }, QSqlDatabase());
}

void MainTableModel::setDisplayHeadInfo(std::vector<QString>& displayHead) {
    this->displayHead = displayHead;
}

// 设定主键是哪个字段
void MainTableModel::setPrimaryKey(QString& key) {
    primaryKey = key;
}

// 根据QModelIndex返回当前行的主键Value
QVariant MainTableModel::getPrimaryKeyValue(const QModelIndex& index) const {
    size_t row = index.row();
    size_t column = 0;
    for(size_t i = 0;i < selectColumns.size() - 1;i++) {
        if(selectColumns[i] == primaryKey) {
            column = i;
            break;
        }
    }
    return tableData[row][column];
}

void MainTableModel::setTableData(std::vector<std::vector<QVariant>>& data) noexcept {
    tableData = data;
}

/**
 * 主要是服务于从redis缓存当中获取数据并模拟复制到该对象时，用于刷新界面
 */
void MainTableModel::updateView() noexcept {
    endResetModel();
}
