#include "datafetch.h"
#include <QNetworkRequest>
#include <iostream>
#include <QUrl>
#include <QTextCodec>
#include <thread>

const QString DataFetch::FETCH_INDEX_URL = "http://hq.sinajs.cn/list=";

void DataFetch::setUrl(QString url) {
    QUrl tempUrl(url);
    QNetworkRequest request(tempUrl);
    reply = netManager.get(request);

    connect(reply, &QIODevice::readyRead, this, &DataFetch::readData);
    connect(reply, &QNetworkReply::finished, this, &DataFetch::turnFinished);
}

void DataFetch::turnFinished() {
    lastFetchFinished = true;
}

void DataFetch::readData() {
    const QByteArray array = reply->readAll();
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

void DataFetch::fetchIndexData(QVector<QString>& codeVector) {
    QString urlStr = FETCH_INDEX_URL;
    if(codeVector.size() > 0) {
        for(int i = 0;i < codeVector.size();i++) {
            QString tempCode = codeVector[i];
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
    }
    connect(&timer, SIGNAL(timeout()), this, SLOT(realFetchIndexData()));
    timer.start(1000);
}
