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

#ifndef NETTEBOOK_NO_KIO
namespace KIO {
class Job;
class TransferJob;
}

class KJob;
#endif

using namespace Qt::StringLiterals;

class IpfsAgent : public QObject
{
    Q_OBJECT
public:
    explicit IpfsAgent(QObject *parent = nullptr);

signals:

public slots:
    QJsonDocument execPost(const QString &suffix, const QString &query, const QJsonDocument &body);

#ifndef NETTEBOOK_NO_KIO
    void getFileKIO(const QUrl &url, std::function<void(QByteArray)> handleResult);

protected slots:
    void fileDataReceived(KIO::Job *job, const QByteArray &data);
    void fileDataReceiveDone(KJob *job);
#endif

private:
    QNetworkAccessManager m_nam;
    QUrl m_apiBaseUrl = QUrl("http://localhost:5001/api/v0/"_L1);
    QByteArray m_userAgent = "nettebook";
    QEventLoop m_eventLoop;
#ifndef NETTEBOOK_NO_KIO
    QHash<QUrl, KJob*> m_resourceLoaders;
    QHash<QUrl, std::function<void (QByteArray)>> m_resourceResponders;
#endif
    QHash<QUrl, QByteArray> m_loadedResources;
};

#endif // IPFSAGENT_H
