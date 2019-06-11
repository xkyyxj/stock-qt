#include "categorytreemodel.h"
#include "datacenter.h"
#include <QSqlDatabase>
#include <QSqlQuery>

extern DataCenter dataCenter;

//-------------------------------TreeItem----------------------------
TreeItem::TreeItem(const Category& data, TreeItem *_parent)
{
    parentItem = _parent;
    itemData = data;
}
//! [0]

//! [1]
TreeItem::~TreeItem()
{
    qDeleteAll(childItems);
}
//! [1]

//! [2]
TreeItem *TreeItem::child(int number)
{
    return childItems.value(number);
}
//! [2]

//! [3]
int TreeItem::childCount() const
{
    return childItems.count();
}
//! [3]

//! [4]
int TreeItem::childNumber() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<TreeItem*>(this));

    return 0;
}
//! [4]

//! [5]
int TreeItem::columnCount() const
{
    return 1;
}
//! [5]

//! [6]
QVariant TreeItem::data(int role) const
{
    switch(role) {
    case Qt::DisplayRole:
        return itemData.categoryName;
    case Qt::EditRole:
        return itemData.categoryName;
    case Category::IDRole:
        return itemData.id;
    case Category::ParentIdRole:
        return itemData.parentId;
    }
    return QVariant();
}
//! [6]

//! [7]
bool TreeItem::insertChildren(int position, Category& data)
{
    if (position < 0 || position > childItems.size())
        return false;

    TreeItem *item = new TreeItem(data, this);
    childItems.insert(position, item);

    return true;
}
//! [7]

//! [9]
TreeItem *TreeItem::parent()
{
    return parentItem;
}
//! [9]

//! [10]
bool TreeItem::removeChildren(int position, int count)
{
    if (position < 0 || position + count > childItems.size())
        return false;

    for (int row = 0; row < count; ++row)
        delete childItems.takeAt(position);

    return true;
}
//! [10]

//! [11]
bool TreeItem::setData(const QVariant &value, int role)
{
    switch(role) {
    case Qt::DisplayRole:
        itemData.categoryName = value.toString();
        break;
    case Qt::EditRole:
        itemData.categoryName = value.toString();
        break;
    case Category::IDRole:
        itemData.id = value.toInt();
        break;
    case Category::ParentIdRole:
        itemData.parentId = value.toInt();
        break;
    default:
        return false;
    }
    return true;
}
//! [11]

//! [13]
static void buildTree(TreeItem& rootItem, QVector<Category> datas) {
    Category temp;
    int position = 0;

    // 构建rootItem的子节点
    QVector<Category>::iterator ite = datas.begin();
    for(;ite != datas.end();) {
        Category temp = *ite;
        if(temp.parentId == rootItem.data(Category::IDRole)) {
            rootItem.insertChildren(position++, temp);
            ite = datas.erase(ite);
        }
        else {
            ++ite;
        }
    }

    // 对rootItem的每个子节点，递归构建子节点
    for(int i = 0;i < rootItem.childItems.size();i++) {
        buildTree(*(rootItem.childItems[i]), datas);
    }
}
//! [13]

//! [12]
// 根据QSqlQuery的查询结果构建一颗树结构
void TreeItem::initByDataResult(QSqlQuery& query) {
    QVector<Category> categories;
    while(query.next()) {
        Category temp;
        temp.id = query.value("pk_category").toInt();
        temp.parentId = query.value("pk_parent").toInt();
        temp.categoryName = query.value("category_name").toString();
        categories.push_back(temp);
    }

    // 递归构建这棵树
    buildTree(*this, categories);
}
//! [12]



// ------------------------TreeModel------------------------------------------
//! [1]
CategoryTreeModel::CategoryTreeModel(QObject *parent): QAbstractItemModel(parent) {
    // 从数据库当中查询数据，然后构建一颗树结构
    rootItem = new TreeItem(Category(), nullptr);
    dataCenter.executeQuery("select * from ana_category", [this](QSqlQuery& query) -> void {
        this->rootItem->initByDataResult(query);
    });
}
//! [1]

//! [2]
CategoryTreeModel::~CategoryTreeModel() {
    delete rootItem;
}

//! [2]
int CategoryTreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return rootItem->columnCount();
}
//! [2]

QVariant CategoryTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::EditRole && role != Category::IDRole)
        return QVariant();

    TreeItem *item = getItem(index);
    if(role == Category::IDRole) {
        return item->data(Category::IDRole);
    }

    return item->data(Qt::DisplayRole);
}

//! [3]
Qt::ItemFlags CategoryTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}
//! [3]

//! [4]
TreeItem *CategoryTreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid()) {
        TreeItem *item = static_cast<TreeItem*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}
//! [4]

QVariant CategoryTreeModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section);

    return QVariant();
}

//! [5]
QModelIndex CategoryTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
        return QModelIndex();
//! [5]

//! [6]
    TreeItem *parentItem = getItem(parent);

    TreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}
//! [6]

//! [7]
QModelIndex CategoryTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    TreeItem *childItem = getItem(index);
    TreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}
//! [7]

//! [8]
int CategoryTreeModel::rowCount(const QModelIndex &parent) const
{
    TreeItem *parentItem = getItem(parent);

    return parentItem->childCount();
}
//! [8]

bool CategoryTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    TreeItem *item = getItem(index);
    bool result = item->setData(value, Qt::DisplayRole);

    if (result)
        emit dataChanged(index, index, {role});

    return result;
}

