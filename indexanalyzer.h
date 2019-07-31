#ifndef INDEXANALYZER_H
#define INDEXANALYZER_H
#include <QSqlDatabase>

class StockIndexBatchInfo;

class IndexAnalyzer {
    QSqlDatabase defaultDatabase;

    bool judgeQuickUp(StockIndexBatchInfo&, double&) noexcept;
    int quickDownThenUp(StockIndexBatchInfo&) noexcept;
public:
    IndexAnalyzer() noexcept;

    [[noreturn]] void operator()() noexcept;

};

#endif // INDEXANALYZER_H
