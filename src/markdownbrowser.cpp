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

#include "application.h"
#include "document.h"
#include "markdownbrowser.h"
#include "settings.h"
#include <QDebug>
#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QMessageBox>
#include <QThread>

MarkdownBrowser::MarkdownBrowser(QWidget *parent)
    : QTextBrowser(parent)
{
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &MarkdownBrowser::onFileChanged);
}

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)

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
    m_loading = name;
    QTextBrowser::setSource(name, type);
}

#else

void MarkdownBrowser::doSetSource(const QUrl &name, QTextDocument::ResourceType type)
{
    const bool newWindow = document()->isModified() || (!m_loading.isEmpty() &&
            Settings::instance()->boolOrDefault(Settings::readingGroup, Settings::openLinksInNewWindows, true));
    qDebug() << this << m_loading << "->" << name << "new window?" << newWindow;
    if (newWindow) {
        static_cast<Application *>(qApp)->load(name);
    } else {
        m_loading = name;
        if (name.fileName().isEmpty()) // directory listing will be markdown
            type = QTextDocument::MarkdownResource;
        QTextBrowser::doSetSource(name, type);
    }
}

#endif

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

void MarkdownBrowser::reload()
{
    static_cast<Document *>(document())->clearCache(m_loading);
    QTextBrowser::reload();
    updateWatcher();
}

void MarkdownBrowser::updateWatcher()
{
    // Some of the cases where this is called may be because saving an existing file
    // is done by creating a new file and deleting the old one.  See QTBUG-53607
    QStringList wasWatching = m_watcher.files();
    if (!wasWatching.isEmpty())
        m_watcher.removePaths(wasWatching);
    if (source().isLocalFile())
        m_watcher.addPath(source().toLocalFile());
    qDebug() << "watching" << m_watcher.files();
}

void MarkdownBrowser::contextMenuEvent(QContextMenuEvent *event)
{
    if (!isReadOnly())
        setTextCursor(cursorForPosition(event->pos()));
    QMenu *menu = createStandardContextMenu(event->pos());
    if (!isReadOnly() && !textCursor().charFormat().anchorHref().isEmpty()) {
        menu->addSeparator();
        QAction *copyLinkAction = menu->addAction(tr("Copy &Link Location"));
        connect(copyLinkAction, &QAction::triggered, this, [this]() {
            QGuiApplication::clipboard()->setText(textCursor().charFormat().anchorHref());
        });
        QAction *editLinkAction = menu->addAction(tr("&Edit Link"));
        connect(editLinkAction, &QAction::triggered, this, [this]() {
            emit MarkdownBrowser::editLink();
        });
        QAction *unlinkAction = menu->addAction(tr("U&nlink"));
        connect(unlinkAction, &QAction::triggered, this, [this]() {
            emit MarkdownBrowser::unlink();
        });
    }
    menu->exec(event->globalPos());
    delete menu;
}

void MarkdownBrowser::wheelEvent(QWheelEvent *event)
{
     if (event->modifiers().testFlag(Qt::ControlModifier) && !isReadOnly()) {
        zoomInF(event->angleDelta().y() / 120.f);
    } else {
        QTextBrowser::wheelEvent(event);
    }
}

void MarkdownBrowser::onFileChanged(const QString &path)
{
    qDebug() << path << "saving?" << static_cast<Document *>(document())->saving();
    if (static_cast<Document *>(document())->saving())
        return;
    if (QMessageBox::question(this, tr("%1 - Reload changed file?").arg(QCoreApplication::applicationName()),
            tr("The file has changed.  Do you want to reload it?")) == QMessageBox::Yes)
        reload();
}
