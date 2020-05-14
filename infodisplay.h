#ifndef INFODISPLAY_H
#define INFODISPLAY_H

#include <QDialog>
#include "data/stockinfo.h"

// 信息显示弹窗，一个小窗口，用来显示选中的K线的信息，日期，收盘价，开盘价等信息
namespace Ui {
class InfoDisplay;
}

class InfoDisplay : public QDialog
{
    Q_OBJECT

public:
    explicit InfoDisplay(QWidget *parent = nullptr);
    ~InfoDisplay();

public slots:
    void stockInfoChanged(StockInfo&) noexcept;
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

    void reject();
private:
    Ui::InfoDisplay *ui;

    bool m_move;
    QPoint m_startPoint;
    QPoint m_windowPoint;
};

#endif // INFODISPLAY_H
