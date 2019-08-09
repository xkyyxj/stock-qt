#ifndef INDEXANALYZER_H
#define INDEXANALYZER_H
#include <QSqlDatabase>
#include <boost/chrono.hpp>
#include "utils/rediscachetools.h"

class StockIndexBatchInfo;

class IndexAnalyzer {
    using milis_t = boost::chrono::time_point<boost::chrono::system_clock, boost::chrono::milliseconds>;

    QSqlDatabase defaultDatabase;
    RedisCacheTools cacheTools;

    void findQuickUp(std::vector<QString>&) noexcept;

    bool judgeQuickUp(StockIndexBatchInfo&, double&) noexcept;
    int quickDownThenUp(StockIndexBatchInfo&) noexcept;

    void analyzeLastMaxUp() noexcept;

public:
    IndexAnalyzer() noexcept;
    IndexAnalyzer(const IndexAnalyzer&) noexcept;

    [[noreturn]] void operator()() noexcept;

};

#endif // INDEXANALYZER_H
