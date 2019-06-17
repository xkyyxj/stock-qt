#pragma execution_character_set("utf-8")
#ifndef DATAFETCH_H
#define DATAFETCH_H

// 定义所有的Boost库都采用动态链接的方式进行链接
#define BOOST_ALL_DYN_LINK

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <boost/chrono.hpp>

using milis_t = boost::chrono::time_point<boost::chrono::system_clock, boost::chrono::milliseconds>;

class DataFetch : public QObject {
    Q_OBJECT
signals:
    void downLoadDone();
private slots:
   void readData();
   void turnFinished();
   void realFetchIndexData();
public:
   static const QString FETCH_INDEX_URL;
public:
    DataFetch(QVector<QString>&& tempVector);

    DataFetch(const DataFetch&);

    void setUrl(QString url);

    void fetchIndexData(QVector<QString>&);

    void operator()();
private:

private:
    QString url;
    QNetworkAccessManager netManager;

    QNetworkReply* reply;

    bool lastFetchFinished = true; // 用于判定上次的拉取数据任务是否完成

    QString currUrl;

    int startIndex = 0; // 共有两千多条信息，一下怼到URL上会导致错误，所以分批次查询

    QVector<QString> codeVector;

    QByteArray remains;

    milis_t preFetchTime;

};

#endif // DATAFETCH_H
