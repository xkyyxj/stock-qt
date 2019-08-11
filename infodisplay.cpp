#include "infodisplay.h"
#include "ui_infodisplay.h"
#include <QMouseEvent>

InfoDisplay::InfoDisplay(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDisplay)
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);
}

InfoDisplay::~InfoDisplay()
{
    delete ui;
}

void InfoDisplay::stockInfoChanged(StockInfo& info) noexcept {
    ui->ts_code->setText(info.ts_code);
    ui->max_price->setText(QString::fromStdString(std::to_string(info.high)));
    ui->min_price->setText(QString::fromStdString(std::to_string(info.low)));
    ui->open_price->setText(QString::fromStdString(std::to_string(info.open)));
    ui->close_price->setText(QString::fromStdString(std::to_string(info.close)));
    ui->pct_chg->setText(QString::fromStdString(std::to_string(info.pct_chg)));
    ui->date->setText(info.trade_date.toString("yyyy-MM-dd"));

    // 计算一下振幅
    float wave = info.pre_close > 0 ?
                (info.high - info.low) / info.pre_close * 100 : -1;
    std::string waveStr = wave > 0 ? std::to_string(wave).append("%") : "0%";
    ui->wave->setText(QString::fromStdString(waveStr));
}

void InfoDisplay::mousePressEvent(QMouseEvent *event)
{
    //当鼠标左键点击时.
    if (event->button() == Qt::LeftButton)
    {
        m_move = true;
        //记录鼠标的世界坐标.
        m_startPoint = event->globalPos();
        //记录窗体的世界坐标.
        m_windowPoint = this->frameGeometry().topLeft();
    }
}
void InfoDisplay::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        //移动中的鼠标位置相对于初始位置的相对位置.
        QPoint relativePos = event->globalPos() - m_startPoint;
        //然后移动窗体即可.
        this->move(m_windowPoint + relativePos );
    }
}
void InfoDisplay::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        //改变移动状态.
        m_move = false;
    }
}

// 空方法，主要是过滤原先的对话框的Esc关闭事件
void InfoDisplay::reject() {}
