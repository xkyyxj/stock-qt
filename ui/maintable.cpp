#include "maintable.h"
#include <iostream>
#include <QAction>
#include "addconcerndialog.h"

MainTable::MainTable()
{
    setContextMenuPolicy(Qt::DefaultContextMenu);
    addConcernDialog = new AddConcernDialog(this);
}

MainTable::MainTable(QSplitter* parent): QTableView (parent) {
    std::cout << "123123" << std::endl;
    setContextMenuPolicy(Qt::DefaultContextMenu);
    addConcernDialog = new AddConcernDialog(this);
}

void MainTable::contextMenuEvent(QContextMenuEvent *) {
    contextMenu = new QMenu(this);
    QAction* addAction = new QAction(this);
    addAction->setText("添加到自选");
    connect(addAction, SIGNAL(triggered()), this, SLOT(addConcernClicked()));
    contextMenu->addAction(addAction);
    contextMenu->exec(QCursor::pos());
}

void MainTable::addConcernClicked() {
    addConcernDialog->show();
}
