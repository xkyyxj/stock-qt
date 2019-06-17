#include "datafetch.h"
#include <QNetworkRequest>
#include <iostream>
#include <QUrl>
#include <QTextCodec>

#include <boost/chrono.hpp>
#include <boost/thread.hpp>

const QString DataFetch::FETCH_INDEX_URL = "http://hq.sinajs.cn/list=";

DataFetch::DataFetch(QVector<QString>&& inputCode) {
    boost::chrono::system_clock::time_point p = boost::chrono::system_clock::now();
    preFetchTime = boost::chrono::time_point_cast<boost::chrono::milliseconds>(p);
    codeVector = inputCode;
}

DataFetch::DataFetch(const DataFetch& pre) {
    this->url = pre.url;
    this->codeVector = pre.codeVector;

    boost::chrono::system_clock::time_point p = boost::chrono::system_clock::now();
    preFetchTime = boost::chrono::time_point_cast<boost::chrono::milliseconds>(p);
}

void DataFetch::operator()() {
    fetchIndexData(codeVector);
}

void DataFetch::setUrl(QString url) {
    QUrl tempUrl(url);
    QNetworkRequest request(tempUrl);
    QNetworkAccessManager tempManager;

    connect(reply, &QIODevice::readyRead, this, &DataFetch::readData);
    connect(reply, &QNetworkReply::finished, this, &DataFetch::turnFinished);
}

void DataFetch::turnFinished() {
    QVector<QString> temp;
    fetchIndexData(temp);
}

void DataFetch::readData() {
    QByteArray array = reply->readAll();
    remains.append(array);
    int lastDivIndex = remains.lastIndexOf(";");
    if(lastDivIndex != -1) {
        // 表明有完整的内容
        array = remains.mid(0, lastDivIndex + 1);
        remains = remains.mid(lastDivIndex);
        // 从新浪财经接口当中返回的数据编码格式为GB18030
        QTextCodec* codec = QTextCodec::codecForName("GB18030");
        QString str = codec->toUnicode(array);
        std::string stdStr = str.toStdString();
        std::cout << boost::this_thread::get_id() << std::endl;
        std::cout << stdStr << std::endl;
    }
}

void DataFetch::realFetchIndexData() {
    if(lastFetchFinished) {
        lastFetchFinished = false;
        setUrl(currUrl);
    }
}

void DataFetch::fetchIndexData(QVector<QString>& codes) {
    using namespace boost::this_thread;
    using namespace boost::chrono;

    if(codes.size() > 0) {
        codeVector = codes;
    }
    QString urlStr = FETCH_INDEX_URL;
    int count = 0;
    if(startIndex >= codeVector.size()) {
        startIndex = 0;
        milis_t currTime = time_point_cast<milliseconds>(system_clock::now());
        milliseconds time_delta = currTime - preFetchTime;
        // 两次获取股票实时数据之间的时间间隔少于1000毫秒
        if(time_delta.count() < 1000) {
            long long target_count = 1000 - time_delta.count();
            milliseconds target_delta(target_count);
            currTime += target_delta;
        }
        sleep_until(currTime);
        preFetchTime = currTime;
    }
    if(codeVector.size() > 0) {
        for(;startIndex < codeVector.size() && count < 330;count++, startIndex++) {
            QString tempCode = codeVector[startIndex];
            if(tempCode.contains("SH")) {
                tempCode.prepend("sh");
            }
            else {
                tempCode.prepend("sz");
            }
            tempCode = tempCode.mid(0, 8);
            urlStr.append(tempCode).append(",");
        }
        currUrl = urlStr;
        setUrl(currUrl);
    }
}
