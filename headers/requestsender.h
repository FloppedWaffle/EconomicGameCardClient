#ifndef REQUESTSENDER_H
#define REQUESTSENDER_H

#include <QObject>
#include <QtNetwork>
#include <functional>
#include <QTimer>



class RequestSender : public QObject
{
    Q_OBJECT
public:
    explicit RequestSender(const QUrl &url, QObject *parent);

    void httpGet(const QString &path, const std::function<void(QNetworkReply *reply)> callback);
    void httpPost(const QString &path, const QJsonObject &data, const std::function<void(QNetworkReply *reply)> callback);



private:
    QUrl m_baseUrl;
    QSettings m_settings;
    QNetworkAccessManager m_networkManager;
};

#endif // REQUESTSENDER_H
