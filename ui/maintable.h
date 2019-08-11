#ifndef MAINTABLE_H
#define MAINTABLE_H

#include <QTableView>
#include <QSplitter>
#include <QMenu>

#include "addconcerndialog.h"
#include "delconcerndialog.h"

class MainTable: public QTableView
{
    Q_OBJECT
    QMenu* contextMenu;
    AddConcernDialog* addConcernDialog;
    DelConcernDialog* delConcernDialog;
public:
    MainTable();
    MainTable(QSplitter*);
public slots:
    void addConcernClicked();
    void delConcernClicked();
protected:
    void contextMenuEvent(QContextMenuEvent *event);
};

#endif // MAINTABLE_H
