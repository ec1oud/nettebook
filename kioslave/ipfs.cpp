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
#include <KIO/ForwardingSlaveBase>
#include <KConfigGroup>

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
    KIO::ForwardingSlaveBase("ipfs", pool, app)
{
    qDebug() << pool << app;
    // http.so doesn't seem to get the message this way
//    setMetaData("cookies", "none");
//    config()->writeEntry("Cookies", false);
}

//void IpfsSlave::get(const QUrl &url)
//{
//    qDebug() << url;
//    mimeType( "text/plain" );
//    QByteArray str( "Hello_world" );
//    data(str);
//    finished();
//    qDebug() << "Leaving function";
//}

bool IpfsSlave::rewriteUrl(const QUrl &url, QUrl& newUrl)
{
    QUrl apiUrl = m_baseUrl;
    apiUrl.setPath(m_baseUrl.path(QUrl::DecodeReserved) + QLatin1String("cat"));
    apiUrl.setQuery("arg=" + url.fileName());
    qDebug() << url << url.fileName() << apiUrl.toString(QUrl::None);
    newUrl = apiUrl;
    return true;
}
