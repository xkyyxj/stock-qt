#include "stockindexfetch.h"

// 定义所有的Boost库都采用动态链接的方式进行链接
#define BOOST_ALL_DYN_LINK
#include <boost/asio/basic_streambuf.hpp>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include <iostream>
//#include <zlib.h>
#include <ctime>

// 两次获取股票基础数据之间的时间价格不少于2000毫秒
static const long TWO_FETCH_DELTA_MILI = 2000;

// 当连续请求了新浪SYNC_TO_REDIS_THRESHOLD次之后，通知将缓存数据刷新到Redis当中
//1000
static int SYNC_TO_REDIS_THRESHOLD = 10;

StockIndexFetch::StockIndexFetch(const StockIndexFetch& origin): context(),socket(context) {
    this->codes = origin.codes;
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

        //330支股票作为一个轮询
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

        //330支股票作为一个轮询
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

    // 初始化一下成交时间段
    boost::gregorian::date currDate(boost::gregorian::day_clock::local_day());
    std::string currDateStr = boost::gregorian::to_iso_extended_string(currDate);
    std::string upBeginTime(currDateStr);
    upBeginTime.append(" 09:30:00");
    //strYtime()
    //upBeginTime = boost
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

        //330支股票作为一个轮询
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

[[noreturn]] void StockIndexFetch::operator()() {
    using namespace boost::this_thread;
    using namespace boost::chrono;

    // 轮询间隔控制变量
    boost::chrono::system_clock::time_point p = boost::chrono::system_clock::now();
    milis_t preFetchTime = boost::chrono::time_point_cast<boost::chrono::milliseconds>(p);

    // 设计为一个死循环
    for(;;) {
        std::string tempS = boost::lexical_cast<std::string>(boost::this_thread::get_id());
        milis_t currTime = time_point_cast<milliseconds>(system_clock::now());
        milliseconds time_delta = currTime - preFetchTime;
        // 两次获取股票实时数据之间的时间间隔少于2000毫秒
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
        /*try {
            http::read(socket, buffer, res);
        }
        catch(std::exception& e) {
            if(currFetchTargetIndex >= urlTargetVec.size()) {
                currFetchTargetIndex = 0;
            }
            std::cout << e.what() << std::endl;
            continue;
        }*/
        http::read(socket, buffer, res);

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

        //std::cout << boost::this_thread::get_id() << std::endl;
        if(ret_result.size() == 0) {
            std::cout << "some thing wrong%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%" << std::endl;
            std::cout << urlTargetVec[currFetchTargetIndex - 1].toStdString() << std::endl;
        }
        //std::cout << ret_result << std::endl;


        //最后修正一下currFetchTargetIndex，以防数组越界
        if(currFetchTargetIndex >= urlTargetVec.size()) {
            currFetchTargetIndex = 0;
        }

        preFetchTime = currTime;
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
