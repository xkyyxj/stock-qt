#include "datafetch.h"
#include <QNetworkRequest>
#include <iostream>
#include <QUrl>
#include <QTextCodec>

const QString DataFetch::FETCH_INDEX_URL = "http://hq.sinajs.cn/list=";

void DataFetch::setUrl(QString url) {
    QUrl tempUrl(url);
    QNetworkRequest request(tempUrl);
    reply = netManager.get(request);

    connect(reply, &QIODevice::readyRead, this, &DataFetch::readData);
    connect(reply, &QNetworkReply::finished, this, &DataFetch::turnFinished);
}

void DataFetch::turnFinished() {
    QVector<QString> temp;
    fetchIndexData(temp);
}

void DataFetch::readData() {
    const QByteArray array = reply->readAll();
    // 从新浪财经接口当中返回的数据编码格式为GB18030
    QTextCodec* codec = QTextCodec::codecForName("GB18030");
    QString str = codec->toUnicode(array);
    std::string stdStr = str.toStdString();
    std::cout << stdStr << std::endl;
}

void DataFetch::realFetchIndexData() {
    if(lastFetchFinished) {
        lastFetchFinished = false;
        setUrl(currUrl);
    }
}

void DataFetch::fetchIndexData(QVector<QString>& codes) {
    if(codes.size() > 0) {
        codeVector = codes;
    }
    QString urlStr = FETCH_INDEX_URL;
    int count = 0;
    if(startIndex >= codeVector.size()) {
        startIndex = 0;
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
    /*connect(&timer, SIGNAL(timeout()), this, SLOT(realFetchIndexData()));
    timer.start(1000);*/
}
