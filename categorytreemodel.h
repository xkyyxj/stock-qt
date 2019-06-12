#pragma execution_character_set("utf-8")
#ifndef CATEGORYTREEMODEL_H
#define CATEGORYTREEMODEL_H

#include <QAbstractItemModel>
#include <QSqlQuery>
#include <QVector>

#include "category.h"

class TreeItem {
public:
    Category itemData;
    QList<TreeItem*> childItems;
    TreeItem* parentItem;

    TreeItem(const Category&, TreeItem*);
    ~TreeItem();

    TreeItem *child(int number);
    int childCount() const;
    int columnCount() const;
    QVariant data(int role) const;
    bool insertChildren(int position, Category& data);
    bool insertColumns(int position, int columns);
    TreeItem *parent();
    bool removeChildren(int position, int count);
    bool removeColumns(int position, int columns);
    int childNumber() const;
    bool setData(const QVariant &value, int role);

    void initByDataResult(QSqlQuery& query);
};

class CategoryTreeModel: public QAbstractItemModel {
public:
    CategoryTreeModel(QObject *parent = 0);
    ~CategoryTreeModel();
//! [0] //! [1]

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
//! [1]
//! [2]
    TreeItem *getItem(const QModelIndex &index) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
//! [2]
private:
    TreeItem *rootItem;
};

#endif // CATEGORYTREEMODEL_H
