#ifndef MAINTABLE_H
#define MAINTABLE_H

#include <QTableView>
#include <QSplitter>
#include <QMenu>

#include "addconcerndialog.h"

class MainTable: public QTableView
{
    Q_OBJECT
    QMenu* contextMenu;
    AddConcernDialog* addConcernDialog;
public:
    MainTable();
    MainTable(QSplitter*);
public slots:
    void addConcernClicked();
protected:
    void contextMenuEvent(QContextMenuEvent *event);
};

#endif // MAINTABLE_H
