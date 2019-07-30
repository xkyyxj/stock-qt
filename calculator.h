﻿#ifndef CALCULATOR_H
#define CALCULATOR_H
#include <vector>
#include <QSqlDatabase>

class StockBaseInfo;

class Calculator {
    QSqlDatabase defaultDatabase;
    std::vector<StockBaseInfo> stockList;

    void initData() noexcept;
public:
    Calculator() noexcept;

    Calculator(const Calculator&) noexcept;

    void findVWaveStock(int) noexcept;

    void findContinueUpStock(int) noexcept;

    void anaIndexInfo() noexcept;

    void operator()() noexcept;

    static void startCalcualte() noexcept;

    void findBigWave(int = 10) noexcept;
};

#endif // CALCULATOR_H
