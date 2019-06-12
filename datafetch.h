#pragma execution_character_set("utf-8")
#ifndef DATAFETCH_H
#define DATAFETCH_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>

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
    void setUrl(QString url);

    void fetchIndexData(QVector<QString>&);
private:

private:
    QString url;
    QNetworkAccessManager netManager;

    QNetworkReply* reply;

    bool lastFetchFinished = true; // 用于判定上次的拉取数据任务是否完成

    QTimer timer;   //定时器，用于循环任务

    QString currUrl;

    int startIndex = 0; // 共有两千多条信息，一下怼到URL上会导致错误，所以分批次查询

    QVector<QString> codeVector;
};

#endif // DATAFETCH_H
