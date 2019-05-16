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

protected:
    void connectTransferJob(KIO::TransferJob *job);

private slots:
    void onDataReceived(KIO::Job *job, const QByteArray &data);
    void onCatReceiveDone(KJob *job);
    void onLsReceiveDone(KJob *job);

private:
    QMimeDatabase m_mimeDb;
    QEventLoop m_eventLoop;
    QByteArray m_dataAccumulator;
    QUrl m_baseUrl = QUrl(QLatin1String("http://localhost:5001/api/v0/"));
    QUrl m_apiUrl;
    QUrl m_fileUrl;
};

#endif // IPFSSLAVE_H
