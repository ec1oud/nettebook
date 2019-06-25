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
#ifndef IPFSAGENT_H
#define IPFSAGENT_H

#include <QEventLoop>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QUrl>

class IpfsAgent : public QObject
{
    Q_OBJECT
public:
    explicit IpfsAgent(QObject *parent = nullptr);

signals:

public slots:
//    void invokeGet(const QString &suffix, const QString &query);
    QJsonDocument execPost(const QString &suffix, const QString &query, const QJsonDocument &body);

private:
    QNetworkAccessManager m_nam;
    QUrl m_apiBaseUrl = QUrl(QLatin1String("http://localhost:5001/api/v0/"));
    QEventLoop m_eventLoop;
};

#endif // IPFSAGENT_H
