/****************************************************************************
**
** Copyright (C) 2019 Shawn Rutledge
**
** This file is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation
** and appearing in the file LICENSE included in the packaging
** of this file.
**
** This code is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
****************************************************************************/
#include "ipfsagent.h"
#include <QHttpPart>
#include <QNetworkReply>

#ifndef NETTEBOOK_NO_KIO
#include <KIO/Job>
#endif

IpfsAgent::IpfsAgent(QObject *parent) : QObject(parent)
{
}

QJsonDocument IpfsAgent::execPost(const QString &suffix, const QString &query, const QJsonDocument &body)
{
    QJsonDocument ret;
    QUrl apiUrl = m_apiBaseUrl;
    apiUrl.setPath(m_apiBaseUrl.path(QUrl::DecodeReserved) + suffix);
    apiUrl.setQuery(query);
    QNetworkRequest req(apiUrl);
    req.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart textPart;
    textPart.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\""));
    textPart.setBody(body.toJson());
    multiPart->append(textPart);
    QNetworkReply *reply = m_nam.post(req, multiPart);
    multiPart->setParent(reply); // delete the multiPart with the reply
    connect(reply, &QNetworkReply::finished, [&]() {
        ret = QJsonDocument::fromJson(reply->readAll());
        m_eventLoop.exit();
    });
    m_eventLoop.exec();
    return ret;
}

#ifndef NETTEBOOK_NO_KIO
void IpfsAgent::getFileKIO(const QUrl &url, std::function<void (QByteArray)> handleResult)
{
qDebug() << "GET" << url;
    KIO::Job* job = KIO::get(url);
    connect (job, SIGNAL(data(KIO::Job *, const QByteArray &)),
             this, SLOT(fileDataReceived(KIO::Job *, const QByteArray &)));
    connect (job, SIGNAL(result(KJob*)), this, SLOT(fileDataReceiveDone(KJob*)));
    m_resourceLoaders.insert(url, job);
    m_resourceResponders.insert(url, handleResult);
}

void IpfsAgent::fileDataReceived(KIO::Job *job, const QByteArray & data)
{
    QUrl url = m_resourceLoaders.key(job);
    // TODO check data for error messages
//qDebug() << job << "received" << data.size() << url;
    if (m_loadedResources.contains(url))
        m_loadedResources[url].append(data);
    else
        m_loadedResources.insert(url, data);
}

void IpfsAgent::fileDataReceiveDone(KJob *job)
{
    QUrl url = m_resourceLoaders.key(job);
    qDebug() << "for" << url.toString() << "got" << m_loadedResources.value(url).size() << "bytes";
    if (!m_loadedResources.value(url).size()) {
        m_loadedResources[url].append(QString());
    }
    m_resourceLoaders.remove(url);
    m_resourceResponders.value(url)(m_loadedResources.value(url));
}
#endif
