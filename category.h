#ifndef CATEGORY_H
#define CATEGORY_H

#include <QObject>
#include <QString>

class Category {
public:
    enum {
        IDRole = Qt::UserRole + 1, ParentIdRole = Qt::UserRole + 2
    };
    QString categoryName;
    int parentId, id;
public:
    Category(): categoryName(""), parentId(-1), id(0) {}
};

Q_DECLARE_METATYPE(Category)
#endif // CATEGORY_H
