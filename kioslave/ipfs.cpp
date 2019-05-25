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

#include "ipfs.h"
#include <QCoreApplication>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <KIO/TransferJob>
#include <KConfigGroup>
#include <KJobUiDelegate>

static const QString base58HashPrefix = QStringLiteral("Qm");
static const QString base32HashPrefix = QStringLiteral("bafybei");

// Pseudo plugin class to embed meta data
class KIOPluginForMetaData : public QObject
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kio.slave.ipfs" FILE "ipfs.json")
};

extern "C" Q_DECL_EXPORT int kdemain(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("kio_ipfs"));

    if (argc != 4) {
        fprintf(stderr, "Usage: kio_ipfs protocol domain-socket1 domain-socket2\n");
        exit(-1);
    }

    IpfsSlave slave(argv[2], argv[3]);
    slave.dispatchLoop();
    return 0;
}

IpfsSlave::IpfsSlave(const QByteArray &pool, const QByteArray &app) :
    KIO::SlaveBase("ipfs", pool, app)
{
    qDebug() << Q_FUNC_INFO << pool << app;
}

void IpfsSlave::get(const QUrl &url)
{
    m_fileUrl = url;
    m_apiUrl = m_baseUrl;
    m_apiUrl.setPath(m_baseUrl.path(QUrl::DecodeReserved) + QLatin1String("cat"));
    QString ipfsPath = url.path();
    if (ipfsPath.startsWith(QLatin1Char('/')))
        ipfsPath = ipfsPath.remove(0, 1);
    m_apiUrl.setQuery("arg=" + ipfsPath);
    qDebug() << Q_FUNC_INFO << url << url.fileName() << m_apiUrl.toString();

    KIO::TransferJob *job = KIO::get(m_apiUrl, KIO::NoReload, KIO::HideProgressInfo);
    connectTransferJob(job);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(onCatReceiveDone(KJob*)));
    m_eventLoop.exec();
//    error(KIO::ERR_DOES_NOT_EXIST, url.toDisplayString());
}

void IpfsSlave::listDir(const QUrl &url)
{
    QString urlString = url.toString(QUrl::RemoveScheme);
    int base58HashIndex = urlString.indexOf(base58HashPrefix);
    int base32HashIndex = urlString.indexOf(base32HashPrefix);
    if (base58HashIndex >= 0)
        urlString = urlString.mid(base58HashIndex);
    else if (base32HashIndex >= 0)
        urlString = urlString.mid(base32HashIndex);
    else while (urlString.startsWith(QLatin1Char('/')))
        urlString = urlString.mid(1);
    m_apiUrl = m_baseUrl;
    m_apiUrl.setPath(m_baseUrl.path(QUrl::DecodeReserved) + QLatin1String("ls"));
    m_apiUrl.setQuery("arg=" + urlString);
    qDebug() << Q_FUNC_INFO << url << urlString << m_apiUrl.toString();
    KIO::TransferJob *job = KIO::get(m_apiUrl, KIO::NoReload, KIO::HideProgressInfo);
    connectTransferJob(job);
    connect(job, SIGNAL(result(KJob*)), this, SLOT(onLsReceiveDone(KJob*)));
    m_eventLoop.exec();
}

void IpfsSlave::connectTransferJob(KIO::TransferJob *job)
{
    connect(job, SIGNAL(data(KIO::Job *, const QByteArray &)),
             this, SLOT(onDataReceived(KIO::Job *, const QByteArray &)));
}

void IpfsSlave::onDataReceived(KIO::Job *job, const QByteArray &data)
{
    Q_UNUSED(job)
    m_dataAccumulator.append(data);
}

void IpfsSlave::onCatReceiveDone(KJob *job)
{
    Q_UNUSED(job)
    QMimeType type = m_mimeDb.mimeTypeForFileNameAndData(m_fileUrl.fileName(), m_dataAccumulator);
    qDebug() << m_fileUrl.fileName() << type;
    mimeType(type.name());
    data(m_dataAccumulator);
    m_dataAccumulator.clear();
    finished();
}

void IpfsSlave::onLsReceiveDone(KJob *job)
{
    if (job->error())
        job->uiDelegate()->showErrorMessage();
    QJsonDocument jdoc = QJsonDocument::fromJson(m_dataAccumulator);
//qDebug() << m_dataAccumulator;
    m_dataAccumulator.clear();
    QJsonArray a = jdoc.object().value(QLatin1String("Objects")).toArray();
    QJsonArray ls = a.first().toObject().value("Links").toArray();
qDebug() << Q_FUNC_INFO << ls.count();
    KIO::UDSEntryList entries;
    for (int i = 0; i < ls.count(); ++i) {
        QJsonObject eo = ls.at(i).toObject();
//        qDebug() << eo;
        KIO::UDSEntry e;
        e.fastInsert(KIO::UDSEntry::UDS_NAME, eo.value(QStringLiteral("Name")).toString());
        e.fastInsert(KIO::UDSEntry::UDS_LINK_DEST, eo.value(QStringLiteral("Hash")).toString());
        e.fastInsert(KIO::UDSEntry::UDS_SIZE, eo.value(QStringLiteral("Size")).toInt());
        // TODO figure out what other file types there are; from experience it looks like 2 is a normal file, 1 is a directory
        e.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, (eo.value(QStringLiteral("Type")).toInt() == 2 ? S_IFREG : S_IFDIR));
        entries << e;
    }
    listEntries(entries);
    finished();
}

#include "ipfs.moc"
