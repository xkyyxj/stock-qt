#ifndef INDEXANALYZER_H
#define INDEXANALYZER_H
#include <QSqlDatabase>

class IndexAnalyzer {
    QSqlDatabase defaultDatabase;
public:
    IndexAnalyzer() noexcept;

    [[noreturn]] void operator()() noexcept;

};

#endif // INDEXANALYZER_H
