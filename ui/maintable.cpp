#include "maintable.h"
#include <iostream>
#include <QAction>
#include "addconcerndialog.h"
#include "maintablemodel.h"

MainTable::MainTable()
{
    setContextMenuPolicy(Qt::DefaultContextMenu);
    addConcernDialog = new AddConcernDialog(this);
    delConcernDialog = new DelConcernDialog(this);
    changeDialog = new HoldingChangeDialog(this);
}

MainTable::MainTable(QSplitter* parent): QTableView (parent) {
    setContextMenuPolicy(Qt::DefaultContextMenu);
    addConcernDialog = new AddConcernDialog(this);
    delConcernDialog = new DelConcernDialog(this);
    changeDialog = new HoldingChangeDialog(this);
}

void MainTable::contextMenuEvent(QContextMenuEvent *) {
    MainTableModel* mainTableModel = static_cast<MainTableModel*>(this->model());
    QString tableName = mainTableModel->getTableName();
    contextMenu = new QMenu(this);
    QAction* addAction = new QAction(this);
    QAction* buyAction = new QAction(this);
    buyAction->setText(tr("买入"));
    QAction* soldAction = new QAction(this);
    soldAction->setText(tr("卖出"));

    connect(buyAction, SIGNAL(triggered()), this, SLOT(changeShareClicked()));
    connect(soldAction, SIGNAL(triggered()), this, SLOT(changeShareClicked()));
    if(tableName == "concern_stock") {
        addAction->setText(tr("删除自选"));
        connect(addAction, SIGNAL(triggered()), this, SLOT(delConcernClicked()));
        contextMenu->addAction(addAction);
        contextMenu->addAction(buyAction);
        contextMenu->addAction(soldAction);
    }
    else {
        addAction->setText(tr("添加到自选"));
        connect(addAction, SIGNAL(triggered()), this, SLOT(addConcernClicked()));
        contextMenu->addAction(addAction);
        contextMenu->addAction(buyAction);
        contextMenu->addAction(soldAction);
    }
    contextMenu->exec(QCursor::pos());
}

void MainTable::addConcernClicked() {
    addConcernDialog->show();
}

void MainTable::delConcernClicked() {
    delConcernDialog->show();
}

void MainTable::changeShareClicked() {
    changeDialog->show();
}
