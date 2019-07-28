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
#include <boost/function.hpp>

using milis_t = boost::chrono::time_point<boost::chrono::system_clock, boost::chrono::milliseconds>;

class StockIndexFetch {
    QVector<QString> codes;
    QVector<QString> urlTargetVec;

    boost::asio::io_context context;
    boost::asio::ip::tcp::socket socket;
    int currFetchTargetIndex;   //当前正在获取urlTargetVec[currFetchTargetIndex]的实时信息

    boost::function<void (std::string&, bool)> writeIndexInfo;

    // 从开始至今fetch的次数，如果是fetch次数达到了
    int fetchTick;

    // 四个开盘以及收盘时间点
    milis_t upBeginTime, upEndTime, downBeginTime, downEndTime;

    bool socketConnected;
public:
    StockIndexFetch(const StockIndexFetch&);
    StockIndexFetch(QVector<QString>&& codes);

    StockIndexFetch(QVector<QString>& codes, boost::function<void (std::string&, bool)>);

    [[noreturn]] void operator()();
private:
    void initSocket(const std::string&);

    void initStratEndTimeP(bool = false) noexcept;
};

#endif // STOCKINDEXFETCHER_H
