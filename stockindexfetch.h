#ifndef STOCKINDEXFETCHER_H
#define STOCKINDEXFETCHER_H

#include <QVector>
#include <QString>

// 定义所有的Boost库都采用动态链接的方式进行链接
#define BOOST_ALL_DYN_LINK
// 禁用标准库的chrono,使用boost.chrono
#define BOOST_ASIO_DISABLE_STD_CHRONO
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>

using milis_t = boost::chrono::time_point<boost::chrono::system_clock, boost::chrono::milliseconds>;

class StockIndexFetch {
    QVector<QString> codes;
    QVector<QString> urlTargetVec;

    boost::asio::io_context context;
    boost::asio::ip::tcp::socket socket;
    int currFetchTargetIndex;   //当前正在获取urlTargetVec[currFetchTargetIndex]的实时信息
public:
    StockIndexFetch(const StockIndexFetch&);
    StockIndexFetch(QVector<QString>&& codes);

    [[noreturn]] void operator()();
private:
    void initSocket(const std::string&);
};

#endif // STOCKINDEXFETCHER_H
