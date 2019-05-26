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

#include "document.h"
#include "markdownbrowser.h"
#include <QDebug>
#include <QApplication>
#include <QThread>

MarkdownBrowser::MarkdownBrowser(QWidget *parent)
    : QTextBrowser(parent)
{

}

QVariant MarkdownBrowser::loadResource(int type, const QUrl &name)
{
    QVariant ret = static_cast<Document *>(document())->loadResource(type, name);
    qDebug() << type << name << (ret.isNull() ? "fetching" : "got it");
    Document *doc = static_cast<Document *>(document());

    QEventLoop loop;
//    connect(doc, SIGNAL(resourceLoaded(QUrl)), &loop, SLOT(onResourceLoaded(QUrl)));
//    connect(&m_loadingTimeout, SIGNAL(timeout()), &loop, SLOT(onLoadingTimeout()));
    connect(doc, SIGNAL(resourceLoaded(QUrl)), &loop, SLOT(quit()));
    connect(&m_loadingTimeout, SIGNAL(timeout()), &loop, SLOT(quit()));
    m_loadingTimeout.start(5000);
    ret = doc->loadResource(type, name);
    if (ret.isNull()) {
        loop.exec();
        ret = doc->loadResource(type, name);
        if (ret.isNull()) {
            switch (type) {
            case QTextDocument::HtmlResource:
                ret = tr("failed to load <a href=\"%1\">%1</a>").arg(name.toString());
                break;
            case QTextDocument::MarkdownResource:
                ret = tr("failed to load [%1](%1)").arg(name.toString());
                break;
            default:
                ret = tr("failed to load %1").arg(name.toString());
                break;
            }
        }
    }
    return ret;
}

void MarkdownBrowser::onResourceLoaded(QUrl url)
{
    qDebug() << url;
}

void MarkdownBrowser::onLoadingTimeout()
{
    qDebug();
}
