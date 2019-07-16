#ifndef CALCULATOR_H
#define CALCULATOR_H
#include <QSqlDatabase>

class Calculator {
    QSqlDatabase defaultDatabase;
public:
    Calculator() noexcept;

    Calculator(const Calculator&) noexcept;

    void findVWaveStock(int) noexcept;

    void findContinueUpStock(int) noexcept;

    void anaIndexInfo() noexcept;

    void operator()() noexcept;

    static void startCalcualte() noexcept;
};

#endif // CALCULATOR_H
