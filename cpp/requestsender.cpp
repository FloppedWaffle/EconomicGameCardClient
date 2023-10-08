#include "requestsender.h"


RequestSender::RequestSender(const QUrl &url, QObject *parent = nullptr)
    : QObject(parent),
    m_baseUrl(url),
    m_settings("EconomicGame", "AuthSettings") {}



void RequestSender::httpGet(const QString &path, const std::function<void(QNetworkReply *reply)> callback)
{
    QUrl requestUrl = m_baseUrl;
    requestUrl.setPath("/" + path);
    QNetworkRequest request(requestUrl);
    request.setRawHeader("Authorization", m_settings.value("token").toString().toUtf8());

    QString token = m_settings.value("token").toString();
    request.setRawHeader("Authorization", token.toUtf8());


    QNetworkReply *reply = m_networkManager.get(request);

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(5000);

    connect(reply, &QNetworkReply::finished, this, [timer, callback, reply]()
    {
        timer->stop();
        timer->deleteLater();
        callback(reply);
        reply->deleteLater();
    });

    connect(timer, &QTimer::timeout, this, [reply]()
    {
        if (reply->isRunning())
        {
            reply->abort();
            reply->deleteLater();
        }
    });
}



void RequestSender::httpPost(const QString &path, const QJsonObject &data, const std::function<void(QNetworkReply *reply)> callback)
{
    QUrl requestUrl = m_baseUrl;
    requestUrl.setPath("/" + path);
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QString token = m_settings.value("token").toString();
    request.setRawHeader("Authorization", token.toUtf8());

    QNetworkReply *reply = m_networkManager.post(request, QJsonDocument(data).toJson());

    QTimer *timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->start(5000);

    connect(reply, &QNetworkReply::finished, this, [timer, callback, reply]()
    {
        timer->stop();
        timer->deleteLater();
        callback(reply);
        reply->deleteLater();
    });

    connect(timer, &QTimer::timeout, this, [reply]()
    {
        if (reply->isRunning())
        {
            reply->abort();
            reply->deleteLater();
        }
    });
}
