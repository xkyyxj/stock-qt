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
};

#endif // DATAFETCH_H
