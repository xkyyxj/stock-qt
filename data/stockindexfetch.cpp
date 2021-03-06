﻿// 定义所有的Boost库都采用动态链接的方式进行链接
#define BOOST_ALL_DYN_LINK
#define BOOST_CHRONO_VERSION 2
#include <boost/asio/basic_streambuf.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/chrono/chrono_io.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>

#include <iostream>
//#include <zlib.h>
#include <ctime>

#include "stockindexfetch.h"

// 两次获取股票基础数据之间的时间价格不少于2000毫秒
static const long TWO_FETCH_DELTA_MILI = 2000;

// 当连续请求了新浪SYNC_TO_REDIS_THRESHOLD次之后，通知将缓存数据刷新到Redis当中
//1000
static int SYNC_TO_REDIS_THRESHOLD = 10;

void logInit() {
    namespace logging = boost::log;
    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );
}

StockIndexFetch::StockIndexFetch(const StockIndexFetch& origin): context(),socket(context) {
    logInit();
    BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";
    this->codes = origin.codes;
    initStratEndTimeP();
    this->writeIndexInfo = origin.writeIndexInfo;
    fetchTick = origin.fetchTick;
    // 初始化需要查询的URL列表
    int count = 0;
    QString tempStr("/list=");
    for(int i = 0;i < codes.size();i++) {
        QString tempCode = codes[i];
        if(tempCode.contains("SH")) {
            tempCode.prepend("sh");
        }
        else {
            tempCode.prepend("sz");
        }
        tempCode = tempCode.mid(0, 8);
        tempStr.append(tempCode).append(",");

        //330支股票作为一个轮询，多了的话ＵＲＬ太长了，会被拒绝访问
        if(count == 329) {
            urlTargetVec.push_back(tempStr);
            tempStr = "/list=";
            count = 0;
        }
        count++;
    }

    if(count > 0) {
        urlTargetVec.push_back(tempStr);
    }

    // 初始化socket链接
    initSocket("hq.sinajs.cn");

    currFetchTargetIndex = 0;
}

StockIndexFetch::StockIndexFetch(QVector<QString>&& codes): context(),socket(context) {
    this->codes = codes;
    initStratEndTimeP();

    // 初始化需要查询的URL列表
    int count = 0;
    fetchTick = 0;
    QString tempStr("/list=");
    for(int i = 0;i < codes.size();i++) {
        QString tempCode = codes[i];
        if(tempCode.contains("SH")) {
            tempCode.prepend("sh");
        }
        else {
            tempCode.prepend("sz");
        }
        tempCode = tempCode.mid(0, 8);
        tempStr.append(tempCode).append(",");

        //330支股票作为一个轮询，多了的话ＵＲＬ太长了，会被拒绝访问
        if(count == 329) {
            urlTargetVec.push_back(tempStr);
            tempStr = "/list=";
            count = 0;
        }
    }

    if(count > 0) {
        urlTargetVec.push_back(tempStr);
    }

    // 初始化socket链接
    initSocket("hq.sinajs.cn");

    currFetchTargetIndex = 0;
}

StockIndexFetch::StockIndexFetch(QVector<QString>& codes,
                                 boost::function<void (std::string&, bool)> fun)
    : context(),socket(context) {
    this->codes = codes;
    writeIndexInfo = fun;
    fetchTick = 0;

    initStratEndTimeP();

    // 初始化需要查询的URL列表
    int count = 0;
    QString tempStr("/list=");
    for(int i = 0;i < codes.size();i++) {
        QString tempCode = codes[i];
        if(tempCode.contains("SH")) {
            tempCode.prepend("sh");
        }
        else {
            tempCode.prepend("sz");
        }
        tempCode = tempCode.mid(0, 8);
        tempStr.append(tempCode).append(",");

        //330支股票作为一个轮询，多了的话ＵＲＬ太长了，会被拒绝访问
        if(count == 329) {
            urlTargetVec.push_back(tempStr);
            tempStr = "/list=";
            count = 0;
        }
    }

    if(count > 0) {
        urlTargetVec.push_back(tempStr);
    }

    // 初始化socket链接
    initSocket("hq.sinajs.cn");

    currFetchTargetIndex = 0;
}

/**
 * 初始化开市以及结束交易的时间点
 */
void StockIndexFetch::initStratEndTimeP(bool isNextDay) noexcept {
    using namespace boost::chrono;
    system_clock::time_point p = system_clock::now();
    // 如果需要延后一天的话，时间戳加上２４小时
    isNextDay ? p += hours(24) : p;

    // 初始化一下开市时间等
    std::time_t currTimeT = system_clock::to_time_t(p);
    struct tm* currTimeP = std::localtime(&currTimeT);

    //上午开市时间 FIXME -- 以前是在开盘前一分钟获取相关信息的，现在因为StockIndexInfo当中存在ＢＵＧ，此处就不早获取数据了
    struct tm temp_tm;
    temp_tm = *currTimeP;   // 结构体的默认拷贝构造
    temp_tm.tm_hour = 9;
    temp_tm.tm_min = 30;
    temp_tm.tm_sec = 0;
    std::time_t time_t = std::mktime(&temp_tm);
    system_clock::time_point tempPoint = system_clock::from_time_t(time_t);
    upBeginTime = time_point_cast<milliseconds>(tempPoint);

    //　上午结束时间
    temp_tm.tm_hour = 11;
    temp_tm.tm_min = 31;
    temp_tm.tm_sec = 0;
    time_t = std::mktime(&temp_tm);
    tempPoint = system_clock::from_time_t(time_t);
    upEndTime = time_point_cast<milliseconds>(tempPoint);

    //　下午开市时间
    temp_tm.tm_hour = 12;
    temp_tm.tm_min = 59;
    temp_tm.tm_sec = 0;
    time_t = std::mktime(&temp_tm);
    tempPoint = system_clock::from_time_t(time_t);
    downBeginTime = time_point_cast<milliseconds>(tempPoint);

    //　下午结束时间
    temp_tm.tm_hour = 15;
    temp_tm.tm_min = 1;
    temp_tm.tm_sec = 0;
    time_t = std::mktime(&temp_tm);
    tempPoint = system_clock::from_time_t(time_t);
    downEndTime = time_point_cast<milliseconds>(tempPoint);
}

[[noreturn]] void StockIndexFetch::operator()() {
    using namespace boost::this_thread;
    using namespace boost::chrono;

    // 轮询间隔控制变量
    boost::chrono::system_clock::time_point p = boost::chrono::system_clock::now();
    milis_t preFetchTime = boost::chrono::time_point_cast<boost::chrono::milliseconds>(p);

    for(;;) {
        milis_t currTime = time_point_cast<milliseconds>(system_clock::now());
        milliseconds time_delta = currTime - preFetchTime;
        // 两次获取股票实时数据之间的时间间隔不少于2000毫秒
        if(time_delta.count() < TWO_FETCH_DELTA_MILI) {
            long long target_count = TWO_FETCH_DELTA_MILI - time_delta.count();
            milliseconds target_delta(target_count);
            currTime += target_delta;
        }
        sleep_until(currTime);
        namespace http = boost::beast::http;
        initSocket("hq.sinajs.cn");
        if(!socketConnected) {
            //std::cout << "socket connect error" << std::endl;
            // socket连接失败，这个循环失效，继续下个循环
            continue;
        }
        std::string currTarget = urlTargetVec[currFetchTargetIndex++].toStdString();
        http::request<http::string_body> req{http::verb::get, currTarget, 11};
        req.set(http::field::host, "hq.sinajs.cn");
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Send the HTTP request to the remote host
        http::write(socket, req);

        // This buffer is used for reading and must be persisted
        boost::beast::flat_buffer buffer;

        // Declare a container to hold the response
        http::response<http::dynamic_body> res;

        // Receive the HTTP response
        try {
            http::read(socket, buffer, res);
        }
        catch(std::exception& e) {
            if(currFetchTargetIndex >= urlTargetVec.size()) {
                currFetchTargetIndex = 0;
            }
            std::cout << e.what() << std::endl;
            continue;
        }
        //http::read(socket, buffer, res);

        //<std::allocator<char>>
        std::string ret_result;
        size_t ret_data_size = 0;
        boost::beast::multi_buffer::const_buffers_type data = res.body().data();
        boost::beast::multi_buffer::const_buffers_type::const_iterator ite = data.begin();
        while(ite != data.end()) {
            auto mu_buffer = *ite;
            ret_data_size += mu_buffer.size();
            std::string data_str(static_cast<const char*>(mu_buffer.data()),mu_buffer.size());
            ret_result.append(data_str);
            ite++;
        }

        // 调用回调函数处理获取的实时信息（并且通知是否刷新Redis）
        writeIndexInfo(ret_result, !(fetchTick++ < SYNC_TO_REDIS_THRESHOLD ? true : fetchTick = 0));

        if(ret_result.size() == 0) {
            std::cout << "some thing wrong%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
            std::cout << urlTargetVec[currFetchTargetIndex - 1].toStdString() << std::endl;
        }


        //最后修正一下currFetchTargetIndex，以防数组越界
        if(currFetchTargetIndex >= urlTargetVec.size()) {
            currFetchTargetIndex = 0;
        }

        preFetchTime = currTime;

        // 判定一下是否交易时间，否则的话，线程睡眠
        //std::cout << time_fmt(boost::chrono::timezone::local) << time_point_cast<seconds>(upBeginTime) << std::endl;
        if(currTime < upBeginTime) {
            sleep_until(upBeginTime);
        }
        else if(currTime > upEndTime && currTime < downBeginTime) {
            sleep_until(downBeginTime);
        }
        else if(currTime > downEndTime) {
            initStratEndTimeP(true);
            sleep_until(upBeginTime);
        }
    }
}

void StockIndexFetch::initSocket(const std::string& host) {
    using boost::asio::ip::tcp;
    socket.close();
    tcp::resolver resolver(context);
    tcp::resolver::query q(host, "80");
    tcp::resolver::iterator endpoint_iterator,end_it;
    try {
        endpoint_iterator = resolver.resolve(q);
    }catch (std::exception&) {

    }
    boost::system::error_code err_code;

    // 如果是像是前一个版本那样写的话，会导致始终加载到最后，然后可能导致连接问题。
    /*
    if(err_code) {
        std::cout << "err1" << std::endl;
    }

    boost::system::error_code err_code2 = boost::asio::error::host_not_found;
    if(err_code2) {
        std::cout << "err2" << std::endl;
    }*/

    boost::asio::ip::tcp::endpoint end_point;
    do {
        end_point = *endpoint_iterator;
        socket.close();
        //boost::asio::connect(socket, end_point);
        socket.connect(end_point, err_code);
        endpoint_iterator++;
    }
    while (err_code && endpoint_iterator != end_it);

    if(err_code) {
        socketConnected = false;
        std:: cout << "socket connect error!" << boost::this_thread::get_id()
                   << err_code.message() << std::endl;        std::cout<< err_code <<std::endl;
    }
    else {
        if(!socketConnected) {
            std::cout << "reconnect success!" << std::endl;
        }
        socketConnected = true;
    }
}
