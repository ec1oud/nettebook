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

void MarkdownBrowser::setSource(const QUrl &name)
{
    m_loading = name;
    // this override doesn't get called from main()
    if (name.fileName().isEmpty()) // directory listing will be markdown
        QTextBrowser::setSource(name, QTextDocument::MarkdownResource);
    else
        QTextBrowser::setSource(name);
}

void MarkdownBrowser::setSource(const QUrl &name, QTextDocument::ResourceType type)
{
    qDebug() << Q_FUNC_INFO << name << type;
    m_loading = name;
    QTextBrowser::setSource(name, type);
}

QVariant MarkdownBrowser::loadResource(int type, const QUrl &name)
{
    Document *doc = static_cast<Document *>(document());
    QVariant ret = doc->loadResource(type, name);
    qDebug() << Q_FUNC_INFO << type << name << (ret.isNull() ? "fetching" : "got it") << "main?" << m_loading;
    if (ret.isNull()) {
        QEventLoop loop;
        connect(doc, SIGNAL(resourceLoaded(QUrl)), &loop, SLOT(quit()));
        connect(&m_loadingTimeout, SIGNAL(timeout()), &loop, SLOT(quit()));
        m_loadingTimeout.start(5000);
        loop.exec();
        ret = doc->loadResource(type, name);
        if (ret.isNull() || doc->status() < Document::NullStatus) {
            if (isReadOnly()) {
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
            qWarning() << ret << doc->errorText();
        }
    }
    return ret;
}
