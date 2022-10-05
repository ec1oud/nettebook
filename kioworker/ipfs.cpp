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
#include <QHttpPart>
#include <QNetworkReply>
#include <KIO/TransferJob>
#include <KConfigGroup>
#include <KJobUiDelegate>

static const QString base58HashPrefix = QStringLiteral("Qm");
static const QString base32HashPrefix = QStringLiteral("baf");
static const QString unixfsPrefix = QStringLiteral("local");
static const QString ipnsPrefix = QStringLiteral("/ipns/");
static const QChar slash = QLatin1Char('/');

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

    IpfsWorker worker(argv[2], argv[3]);
    worker.dispatchLoop();
    return 0;
}

IpfsWorker::IpfsWorker(const QByteArray &pool, const QByteArray &app) :
    KIO::SlaveBase("ipfs", pool, app)
{
    qDebug() << Q_FUNC_INFO << pool << app;
}

void IpfsWorker::get(const QUrl &url)
{
    m_fileUrl = url;
    QString urlString = apiPath(url);
    if (urlString.isEmpty()) {
        error(KIO::ERR_DOES_NOT_EXIST, url.toString());
        return;
    }
    bool unixFs = urlString.startsWith(slash + unixfsPrefix);
    if (unixFs)
        urlString = urlString.mid(6);
    m_apiUrl = m_baseUrl;
    m_apiUrl.setPath(m_baseUrl.path(QUrl::DecodeReserved) + (unixFs ?  QLatin1String("files/read") : QLatin1String("cat")));
    m_apiUrl.setQuery("arg=" + urlString);
    qDebug() << Q_FUNC_INFO << url << url.fileName() << m_apiUrl.toString();
    QNetworkRequest req(m_apiUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, m_defaultContentType);
    req.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
    m_reply = m_nam.post(req, QByteArray());
    connect(m_reply, &QNetworkReply::finished, this, &IpfsWorker::onCatReceiveDone);
    m_eventLoop.exec();
}

void IpfsWorker::listDir(const QUrl &url)
{
    m_fileUrl = url;
    QString urlString = apiPath(url);
    bool unixFs = urlString.startsWith(slash + unixfsPrefix);
    if (unixFs)
        urlString = urlString.mid(6);
    m_apiUrl = m_baseUrl;
    // files/ls happens to work for both local unixfs and for hashes
    m_apiUrl.setPath(m_baseUrl.path(QUrl::DecodeReserved) +
                     (unixFs ? QLatin1String("files/ls") : QLatin1String("ls")));

    m_apiUrl.setQuery(QLatin1String("arg=") + (urlString.isEmpty() ? "/" : urlString) + m_querySuffix);
    qDebug() << Q_FUNC_INFO << url << urlString << m_apiUrl.toString();
    QNetworkRequest req(m_apiUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, m_defaultContentType);
    req.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
    m_reply = m_nam.post(req, QByteArray());
    connect(m_reply, &QNetworkReply::finished, this, &IpfsWorker::onLsReceiveDone);
    m_eventLoop.exec();
}

void IpfsWorker::stat(const QUrl &url)
{
    m_fileUrl = url;
    QString urlString = apiPath(url);
    if (urlString.isEmpty()) {
        error(KIO::ERR_DOES_NOT_EXIST, url.toString());
        return;
    }
    bool unixFs = urlString.startsWith(slash + unixfsPrefix);
    if (unixFs)
        urlString = urlString.mid(6);
    m_apiUrl = m_baseUrl;
    // files/stat may have been intended for unixfs, but happens to work just as well with hashes.
    // It always provides more info than block/stat, and block/stat doesn't work with unixfs.
    m_apiUrl.setPath(m_baseUrl.path(QUrl::DecodeReserved) + QLatin1String("files/stat"));
    m_apiUrl.setQuery((unixFs ? "arg=/" : "arg=/ipfs/") + urlString + m_querySuffix);
    qDebug() << Q_FUNC_INFO << this << url << urlString << m_apiUrl.toString();
    QNetworkRequest req(m_apiUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, m_defaultContentType);
    req.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
    m_reply = m_nam.post(req, QByteArray());
    connect(m_reply, &QNetworkReply::finished, this, &IpfsWorker::onStatReceiveDone);
    m_eventLoop.exec();
}

void IpfsWorker::put(const QUrl &url, int permissions, KIO::JobFlags flags)
{
    m_fileUrl = url;
    QString urlString = apiPath(url);
qDebug() << url << urlString << permissions << flags;
    m_newObject = false;
    if (urlString.isEmpty()) {
        if (url.scheme() == QLatin1String("ipfs")) {
            m_apiUrl = m_baseUrl;
            m_apiUrl.setPath(m_baseUrl.path(QUrl::DecodeReserved) + QLatin1String("add"));
            m_apiUrl.setQuery(QLatin1String("cid-version=1"));
            qDebug() << "writing new object" << m_apiUrl;
            m_newObject = true;
        } else {
            error(KIO::ERR_DOES_NOT_EXIST, url.toString());
            return;
        }
    } else {
        bool unixFs = urlString.startsWith(slash + unixfsPrefix);
        if (unixFs)
            urlString = urlString.mid(6);
        m_apiUrl = m_baseUrl;
        m_apiUrl.setPath(m_baseUrl.path(QUrl::DecodeReserved) + QLatin1String("files/write"));
        m_apiUrl.setQuery("arg=" + urlString + "&create=true&truncate=true&cid-version=1");
    }

    QByteArray acc;
    int result;
    // Loop until we got 0 (end of data)
    do {
        QByteArray buffer;
        dataReq();
        result = readData(buffer);
        acc.append(buffer);
    } while (result > 0);
    qDebug() << Q_FUNC_INFO << url << permissions << flags << urlString << m_apiUrl.toString()
             << "type" << metaData("content-type") << "len" << acc.length();
//    qDebug() << allMetaData();

    if (result < 0) {
        qDebug() << Q_FUNC_INFO << "error";
        ::exit(255);
    }
    if (!acc.length()) {
        qDebug() << Q_FUNC_INFO << "no data to write";
        ::exit(255);
    }

    QNetworkRequest req(m_apiUrl);
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    QHttpPart textPart;
    QString type = metaData("content-type");
    if (type.isEmpty())
        type = QLatin1String("text/plain");
    textPart.setHeader(QNetworkRequest::ContentTypeHeader, type);
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\""));
    req.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
    textPart.setBody(acc);
    multiPart->append(textPart);
    m_reply = m_nam.post(req, multiPart);
    multiPart->setParent(m_reply); // delete the multiPart with the reply
    connect(m_reply, &QNetworkReply::finished, this, &IpfsWorker::onPutDone);
    m_eventLoop.exec();
}

void IpfsWorker::onCatReceiveDone()
{
    m_eventLoop.exit();
    if (m_reply->error()) {
        qDebug() << Q_FUNC_INFO << m_reply->errorString();
        error(KIO::ERR_SLAVE_DEFINED, m_reply->errorString());
        return;
    }
    QByteArray buf = m_reply->readAll();
    m_reply->deleteLater();
    m_reply = nullptr;
    QMimeType type = m_mimeDb.mimeTypeForFileNameAndData(m_fileUrl.fileName(), buf);
    qDebug() << m_fileUrl.fileName() << type;
    mimeType(type.name());
    data(buf);
    finished();
}

void IpfsWorker::onLsReceiveDone()
{
    m_eventLoop.exit();
    if (m_reply->error()) {
        qDebug() << Q_FUNC_INFO << m_reply->errorString();
        error(KIO::ERR_SLAVE_DEFINED, m_reply->errorString());
        return;
    }
    QJsonDocument jdoc = QJsonDocument::fromJson(m_reply->readAll());
    m_reply->deleteLater();
    m_reply = nullptr;
    QJsonArray ls;
    if (jdoc.object().value(QLatin1String("Objects")) != QJsonValue::Undefined) {
        QJsonArray a = jdoc.object().value(QLatin1String("Objects")).toArray();
        ls = a.first().toObject().value("Links").toArray();
    } else if (jdoc.object().value(QLatin1String("Entries")) != QJsonValue::Undefined) {
        ls = jdoc.object().value(QLatin1String("Entries")).toArray();
    }
//qDebug() << Q_FUNC_INFO << m_fileUrl << ls.count();
    KIO::UDSEntryList entries;
    for (int i = 0; i < ls.count(); ++i) {
        QJsonObject eo = ls.at(i).toObject();
        bool isDir = (eo.value(QStringLiteral("Type")).toInt() == 1);
        QString name = eo.value(QStringLiteral("Name")).toString();
//        if (isDir && !name.endsWith(slash))
//            name.append(slash);
//qDebug() << Q_FUNC_INFO << "   " << i << (isDir ? "dir" : "file") << eo;
        KIO::UDSEntry e;
        e.reserve(5);
        e.fastInsert(KIO::UDSEntry::UDS_NAME, name);
        e.fastInsert(KIO::UDSEntry::UDS_LINK_DEST, eo.value(QStringLiteral("Hash")).toString());
        e.fastInsert(KIO::UDSEntry::UDS_SIZE, eo.value(QStringLiteral("Size")).toInt());
        // TODO figure out what other file types there are; from experience it looks like 2 is a normal file, 1 is a directory
        e.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, isDir ? S_IFDIR : S_IFREG);
        e.fastInsert(KIO::UDSEntry::UDS_ACCESS, isDir ? S_IRWXU | S_IRWXG | S_IRWXO : S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
        entries << e;
    }
    listEntries(entries);
    finished();
}

void IpfsWorker::onStatReceiveDone()
{
    m_eventLoop.exit();
    if (m_reply->error()) {
        qDebug() << Q_FUNC_INFO << m_reply->errorString();
        error(KIO::ERR_SLAVE_DEFINED, m_reply->errorString());
        return;
    }
    QJsonObject jo = QJsonDocument::fromJson(m_reply->readAll()).object();
    m_reply->deleteLater();
    m_reply = nullptr;
    QJsonValue msg = jo.value(QLatin1String("Message"));
    if (!msg.isUndefined()) {
        int code = jo.value(QStringLiteral("Code")).toInt();
        switch (code) {
        case 0: // for ipfs:/// : "path must contain at least one component"
            error(KIO::ERR_IS_DIRECTORY, msg.toString());
            return;
        default:
            error(KIO::ERR_SLAVE_DEFINED, msg.toString());
            return;
        }
    }
    bool isDir = (jo.value(QStringLiteral("Type")).toString() == QLatin1String("directory"));
//    qDebug() << Q_FUNC_INFO << this << m_fileUrl << (isDir ? "dir" : "file") << jo;
    KIO::UDSEntry e;
    e.reserve(4);
    e.fastInsert(KIO::UDSEntry::UDS_NAME, m_fileUrl.toString());
    e.fastInsert(KIO::UDSEntry::UDS_SIZE, jo.value(QStringLiteral("Size")).toInt()); // or CumulativeSize?
    e.fastInsert(KIO::UDSEntry::UDS_FILE_TYPE, isDir ? S_IFDIR : S_IFREG);
    e.fastInsert(KIO::UDSEntry::UDS_ACCESS, isDir ? S_IRWXU | S_IRWXG | S_IRWXO : S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    // TODO there should be more
    statEntry(e);
    finished();
}

void IpfsWorker::onPutDone()
{
    if (m_reply->error()) {
        qWarning() << Q_FUNC_INFO << m_reply->errorString();
        error(KIO::ERR_SLAVE_DEFINED, m_reply->errorString());
    } else {
        qDebug() << Q_FUNC_INFO << m_reply->rawHeaderPairs(); // << m_reply->rawHeaderList();
        for (auto h : m_reply->rawHeaderList())
            qDebug() << h << m_reply->rawHeader(h);
        QJsonDocument doc = QJsonDocument::fromJson(m_reply->readAll());
        qDebug() << doc.object();
        if (m_newObject) {
            setMetaData(QLatin1String("Hash"), doc.object().value(QLatin1String("Hash")).toString());
            emit infoMessage(QLatin1String("Saved ") + doc.object().value(QLatin1String("Hash")).toString());
        }
        finished();
    }
    m_reply->deleteLater();
    m_reply = nullptr;
    m_eventLoop.exit();
}

QString IpfsWorker::ipnsLookup(const QString &path)
{
    m_apiUrl = m_baseUrl;
    // http://localhost:5001/api/v0/name/resolve?arg=<name>&recursive=true&nocache=<value>&dht-record-count=<value>&dht-timeout=<value>&stream=<value>
    m_apiUrl.setPath(m_baseUrl.path(QUrl::DecodeReserved) + QLatin1String("name/resolve"));
    m_apiUrl.setQuery("arg=" + path + "&dht-timeout=3s");
    qDebug() << Q_FUNC_INFO << path << m_apiUrl.toString();
    QNetworkRequest req(m_apiUrl);
    req.setHeader(QNetworkRequest::ContentTypeHeader, m_defaultContentType);
    req.setHeader(QNetworkRequest::UserAgentHeader, m_userAgent);
    m_reply = m_nam.post(req, QByteArray());
    connect(m_reply, &QNetworkReply::finished, [=]() { m_eventLoop.exit(); } );
    m_eventLoop.exec();
    QJsonObject jo = QJsonDocument::fromJson(m_reply->readAll()).object();
    m_reply->deleteLater();
    m_reply = nullptr;
    qDebug() << jo;
    QJsonValue ret = jo.value(QLatin1String("Path"));
    if (ret.isUndefined())
        return QString();
    return ret.toString();
}

QString IpfsWorker::apiPath(const QUrl &url)
{
    QString urlString = url.toString(QUrl::RemoveScheme);
    int ipnsPrefixIndex = urlString.indexOf(ipnsPrefix);
    if (ipnsPrefixIndex >= 0) {
        urlString = urlString.mid(ipnsPrefixIndex);
        // CIDv1 doesn't actually work for ipns yet, but let's hope it will eventually
        if (urlString.indexOf(base32HashPrefix, ipnsPrefixIndex) > 0 ||
                urlString.indexOf(base58HashPrefix, ipnsPrefixIndex) > 0)
            urlString = ipnsLookup(urlString);
    }
    int base58HashIndex = urlString.indexOf(base58HashPrefix);
    int base32HashIndex = urlString.indexOf(base32HashPrefix);
    int unixFsPrefixIndex = urlString.indexOf(unixfsPrefix);
    if (base58HashIndex >= 0)
        urlString = urlString.mid(base58HashIndex);
    else if (base32HashIndex >= 0)
        urlString = urlString.mid(base32HashIndex);
    else if (unixFsPrefixIndex >= 0)
        urlString = slash + urlString.mid(unixFsPrefixIndex);
    else while (urlString.startsWith(slash))
        urlString = urlString.mid(1);
    return urlString;
}

#include "ipfs.moc"
