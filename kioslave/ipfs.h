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

#ifndef IPFSSLAVE_H
#define IPFSSLAVE_H

#include <KIO/SlaveBase>
#include <QEventLoop>
#include <QMimeDatabase>
#include <QNetworkAccessManager>

namespace KIO {
class TransferJob;
}

class IpfsSlave : public QObject, public KIO::SlaveBase
{
    Q_OBJECT
public:
    IpfsSlave(const QByteArray &pool, const QByteArray &app);
    void get(const QUrl &url) override;
    void listDir(const QUrl &url) override;
    void stat(const QUrl &url) override;
    void put(const QUrl &url, int permissions, KIO::JobFlags flags) override;

private slots:
    void onCatReceiveDone();
    void onLsReceiveDone();
    void onStatReceiveDone();
    void onPutDone();

private:
    QString ipnsLookup(const QString &path);
    QString apiPath(const QUrl &url);

private:
    QMimeDatabase m_mimeDb;
    QEventLoop m_eventLoop;
    QUrl m_baseUrl = QUrl(QLatin1String("http://localhost:5001/api/v0/"));
    QUrl m_apiUrl;
    QUrl m_fileUrl;
    QNetworkAccessManager m_nam;
    QNetworkReply *m_reply = nullptr;
};

#endif // IPFSSLAVE_H
