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
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QScrollBar>
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
    m_loading = name;
    if (name.fileName().isEmpty()) // directory listing will be markdown
        type = QTextDocument::MarkdownResource;
    QTextBrowser::doSetSource(name, type);
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

QList<MarkdownBrowser::LinkInfo> MarkdownBrowser::viewportLinks()
{
    QList<MarkdownBrowser::LinkInfo> ret;
    const int verticalPos = verticalScrollBar()->value();
    const int vpHeight = viewport()->height();
    for (QTextBlock block = document()->begin(); block != document()->end(); block = block.next()) {
        for (auto fragIter = block.begin(); !fragIter.atEnd(); ++fragIter) {
            const QTextFragment frag = fragIter.fragment();
            const QTextCharFormat fmt = frag.charFormat();
            const QStringList anames = fmt.anchorNames();
            QTextLayout *layout = block.layout();
            if (!anames.isEmpty() || fmt.isAnchor()) {
                LinkInfo linfo;
                QList<QGlyphRun> runs = frag.glyphRuns();
                bool relevant = false;
                for (const QGlyphRun &glrun : runs) {
                    QRectF bounds = glrun.boundingRect();
                    bounds.translate(layout->position());
                    bounds.translate(0, -verticalPos);
                    if (bounds.bottom() > 0 && bounds.top() < vpHeight)
                        relevant = true;
                    linfo.appendRegion(bounds);
                }
                if (!relevant)
                    continue;
                if (!anames.isEmpty()) {
                    linfo.linkOrAnchorName.setFragment(anames.first());
                    qDebug() << "hyperlink destination" << linfo;
                } else if (fmt.isAnchor()) {
                    linfo.linkOrAnchorName = QUrl(fmt.anchorHref());
                    qDebug() << "hyperlink source" << frag.text() << linfo;
                }
                ret << linfo;
            }
        }
    }
    return ret;
}

void MarkdownBrowser::paintEvent(QPaintEvent *e)
{
    QList<MarkdownBrowser::LinkInfo> links = viewportLinks();
    QPainter p(viewport());
    p.setPen(Qt::red);
    for (const auto &linfo : links) {
        for (const auto &poly : linfo.region)
            p.drawPolygon(poly);
        p.drawText(linfo.region.first().first(), linfo.linkOrAnchorName.toString());
    }
    QTextBrowser::paintEvent(e);
}

void MarkdownBrowser::contextMenuEvent(QContextMenuEvent *event)
{
    if (!isReadOnly())
        setTextCursor(cursorForPosition(event->pos()));
    QMenu *menu = createStandardContextMenu(event->pos());
    if (!isReadOnly() && !textCursor().charFormat().anchorHref().isEmpty()) {
        menu->addSeparator();
        QAction *editLinkAction = menu->addAction(tr("Edit Link"));
        connect(editLinkAction, &QAction::triggered, this, [this]() {
            emit MarkdownBrowser::editLink();
        });
        QAction *unlinkAction = menu->addAction(tr("Unlink"));
        connect(unlinkAction, &QAction::triggered, this, [this]() {
            emit MarkdownBrowser::unlink();
        });
    }
    menu->exec(event->globalPos());
    delete menu;
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

static bool overlappingRange(qreal l1, qreal r1, qreal l2, qreal r2)
{
    // l1 is between l2 and r2 or
    // r1 is between l2 and r2
    return ((l1 >= l2 && l1 <= r2) || (r1 >= l2 && r1 <= r2));
}

void MarkdownBrowser::LinkInfo::appendRegion(QRectF p)
{
    if (!region.isEmpty()) {
        QPolygonF &last = region.last();
        const auto &lastBounds = last.boundingRect();
        const qreal dy = qAbs(lastBounds.bottom() - p.top());
        if (dy < 3 && overlappingRange(p.left(), p.right(), lastBounds.left(), lastBounds.right())) {
            p.moveTop(p.top() - dy - 0.1);
            region << region.takeLast().united(p);
            return;
        }
    }
    region << p;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug dbg, const MarkdownBrowser::LinkInfo &info)
{
    QDebugStateSaver saver(dbg);
    dbg << info.linkOrAnchorName << info.region << "first y" << info.region.first().first().y();
    return dbg;
}
#endif // !QT_NO_DEBUG_STREAM
