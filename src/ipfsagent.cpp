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
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart textPart;
    textPart.setHeader(QNetworkRequest::ContentTypeHeader, QLatin1String("application/json"));
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\""));
    textPart.setBody(body.toJson());
    multiPart->append(textPart);
    QNetworkReply *m_reply = m_nam.post(req, multiPart);
    multiPart->setParent(m_reply); // delete the multiPart with the reply
    connect(m_reply, &QNetworkReply::finished, [&]() {
        ret = QJsonDocument::fromJson(m_reply->readAll());
        m_eventLoop.exit();
    });
    m_eventLoop.exec();
    return ret;
}
