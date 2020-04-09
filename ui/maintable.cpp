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
    int currRow = this->currentIndex().row();
    MainTableModel* mainTableModel = static_cast<MainTableModel*>(this->model());
    QString tableName = mainTableModel->getTableName();
    QModelIndex codeIndex = mainTableModel->index(currRow,0);
    QString ts_code = mainTableModel->data(codeIndex, PRIMARY_KEY_ROLE).toString();
    contextMenu = new QMenu(this);
    QAction* addAction = new QAction(this);
    QAction* buyAction = new QAction(this);
    buyAction->setText(tr("买入"));
    QAction* soldAction = new QAction(this);
    soldAction->setText(tr("卖出"));

    connect(buyAction, SIGNAL(triggered()), this, SLOT(changeShareClicked()));
    connect(soldAction, SIGNAL(triggered()), this, SLOT(changeShareClicked()));
    if(tableName == "daily_concern") {
        // 下面这段意思是：如果当前显示的是自选，那么就从自选当中删除
        addAction->setText(tr("删除自选"));
        connect(addAction, SIGNAL(triggered()), this, SLOT(delConcernClicked()));
        contextMenu->addAction(addAction);
        contextMenu->addAction(buyAction);
        contextMenu->addAction(soldAction);
    }
    else {
        // 如果显示的是其他计算得出的股票，那么可以加入到自选当中
        addConcernDialog->setTsCode(ts_code);
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
