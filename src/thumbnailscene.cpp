/****************************************************************************
** This file is part of the nettebook project hosted at
** github.com/ec1oud/nettebook
** Copyright (C) 2019 Shawn Rutledge
** This file is also part of the taborca project hosted at
** sf.net/projects/taborca
** Copyright (C) 2013 Shawn Rutledge
** Contact: s@ecloud.org
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
****************************************************************************/
#include "thumbnailscene.h"
#include "ipfsagent.h"
#include <KIO/TransferJob>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QPainter>

#ifndef NETTEBOOK_NO_KIO
#include <KIO/Job>
#endif

ThumbnailScene::ThumbnailScene() :
    QGraphicsScene()
{
    qDebug("ThumbnailScene constructor");
    setBackgroundBrush(QBrush(Qt::darkGray));
    QPen thickRed(Qt::red);
    thickRed.setWidth(4);
    insertionPoint.setPen(thickRed);
    QAction *newAction = m_contextMenu.addAction(tr("New"));
    connect(newAction, &QAction::triggered, this, &ThumbnailScene::appendBlank);
    connect(this, &QGraphicsScene::selectionChanged, this, &ThumbnailScene::selectionChanged);
}

void ThumbnailScene::resize(int numItems)
{
    qDeleteAll(items); // TODO dangling pointers?
    items.resize(numItems);
    col = 0;
    row = 0;
    maxWidth = 0;
    maxHeight = 0;
    selectedIdx = -1;
}

void ThumbnailScene::clear()
{
    qDeleteAll(items);
    items.clear();
}

void ThumbnailScene::appendBlank()
{
    QString label = tr("page %1").arg(items.count());
    ThumbnailItem* item = new ThumbnailItem(items.count(), label);
    item->content = label;
    addItem(item);
    maxWidth = qMax(int(item->boundingRect().width()), maxWidth);
    maxHeight = qMax(int(item->boundingRect().height()), maxHeight);
    items.append(item);
    item->setPos(col++ * maxWidth + spacing, row * maxHeight + spacing);
    if (col >= cols) {
        col = 0;
        ++row;
    }
    selectedIdx = items.count() - 1;
}

void ThumbnailScene::append(const QString &ipfsPath, const QString &content)
{
    ThumbnailItem* item = new ThumbnailItem(items.count(), ipfsPath);
    item->content = content;
    addItem(item);
    maxWidth = qMax(int(item->boundingRect().width()), maxWidth);
    maxHeight = qMax(int(item->boundingRect().height()), maxHeight);
    items.append(item);
    item->setPos(col++ * maxWidth + spacing, row * maxHeight + spacing);
    if (col >= cols) {
        col = 0;
        ++row;
    }
//    selectedIdx = items.count() - 1;
}

void ThumbnailScene::layout(int width, int height)
{
    col = 0;
    row = 0;
    if (width > 0 && maxWidth > 0)
        cols = (width + spacing) / (maxWidth + spacing);
    //qDebug("layout(%d), cols now %d, sceneRect %f x %f", width, cols, sceneRect().width(), sceneRect().height());
    int i = 0;
    for (ThumbnailItem* item : items) {
        if (col >= cols) {
            col = 0;
            ++row;
        }
        item->setPos(col++ * (maxWidth + spacing) + (maxWidth - int(item->boundingRect().width())) / 2,
                     row * (maxHeight + spacing) + (maxHeight - int(item->boundingRect().height())) / 2);
        ++i;
    }
    QRectF sceneBounds = sceneRect();
    if (height > 0)
        sceneBounds.setHeight(qMax(height, (row + 1) * (maxHeight + spacing)));
    if (width > 0)
        sceneBounds.setWidth(width);
    setSceneRect(sceneBounds);
}

void ThumbnailScene::setCurrentPage(ThumbnailItem *item)
{
    emit currentPageChanging(items[selectedIdx]);
    selectedIdx = item->pageId;
	update(sceneRect());
    emit currentPageThumbnail(item);
    emit currentPageChanged(item->label, item->content);
}

void ThumbnailScene::dragEnterEvent ( QGraphicsSceneDragDropEvent * ev )
{
    qDebug("dragEnterEvent at %f, %f", ev->scenePos().x(), ev->scenePos().y());
    ev->setAccepted(true);
    addItem(&insertionPoint);
}

void ThumbnailScene::dragMoveEvent ( QGraphicsSceneDragDropEvent * ev )
{
//	qDebug("dragMoveEvent to %f, %f", ev->scenePos().x(), ev->scenePos().y());
    QPoint insColRow = insertionIdx(ev->scenePos());
    int x = (insColRow.x() + 1) * (maxWidth + spacing);// + (spacing / 2);
    if (x == 0)
        x = insertionPoint.pen().width() / 2;
    else
        x -= insertionPoint.pen().width() / 2;
    int y = insColRow.y() * (maxHeight + spacing) + (spacing / 2);
//	qDebug("line top at %d, %d", x, y);
	insertionPoint.setLine(x, y, x, y + maxHeight);
	int insIdx = insColRow.y() * cols + insColRow.x();
    if (insIdx >= 0) {
		if (insIdx + 1 < items.size())
            emit statusMessage(QString("moving between %1 and %2").arg(items[insIdx]->pageId).arg(items[insIdx + 1]->pageId));
		else
			emit statusMessage(QString("moving to end"));
    } else {
		emit statusMessage("moving to beginning");
    }
}

void ThumbnailScene::dropEvent ( QGraphicsSceneDragDropEvent * ev )
{
	qDebug("dropEvent at %f, %f", ev->scenePos().x(), ev->scenePos().y());
	removeItem(&insertionPoint);
	emit statusClear();
	QPoint insColRow = insertionIdx(ev->scenePos());
    const QList<QGraphicsItem *> sel = selectedItems();
	int insIdx = insColRow.y() * cols + insColRow.x() + 1;
    for (QGraphicsItem* item : sel) {
        int oldIdx = items.indexOf(static_cast<ThumbnailItem*>(item));
        if (oldIdx < 0)
            qWarning("item being dropped is not in the set of thumbnails!");
        else
            items.remove(oldIdx);
        if (oldIdx < insIdx)
            --insIdx;
    }
    for (QGraphicsItem* item : sel)
        items.insert(insIdx++, static_cast<ThumbnailItem*>(item));
    layout();
}

QPoint ThumbnailScene::insertionIdx(QPointF pos)
{
    int row = int(pos.y() / (maxHeight + spacing));
    int xadj = int(pos.x() - ((maxWidth + spacing) / 2));
    int col = (xadj < 0 ? -1 : xadj / maxWidth);
    return QPoint(col, row);
}

void ThumbnailScene::selectionChanged()
{
    const QList<QGraphicsItem *> sel = selectedItems();
    if (sel.size() > 0)
        setCurrentPage(static_cast<ThumbnailItem*>(sel.first()));
}

void ThumbnailScene::saveAllToIpfs()
{
    for (ThumbnailItem * item : items) {
        item->saved = false;
#ifndef NETTEBOOK_NO_KIO
        QUrl url("ipfs:///");
        KIO::TransferJob *job = KIO::put(url, -1, KIO::Overwrite);
        job->addMetaData("content-type", QLatin1String("text/markdown"));
        m_saveJobs.insert(job, item);
        connect (job, &KIO::TransferJob::dataReq, [=](KIO::Job *job, QByteArray &dest) {
            ThumbnailItem *item = m_saveJobs.value(job);
            if (!item || item->saved)
                return;
            dest = item->content.toUtf8();
            item->saved = true;
        });
        connect (job, SIGNAL(result(KJob*)), this, SLOT(saveJobResult(KJob*)));
#endif
    }
}

#ifndef NETTEBOOK_NO_KIO
void ThumbnailScene::saveJobResult(KJob *job)
{
    QString hash = static_cast<KIO::Job *>(job)->metaData().value(QLatin1String("Hash"), QString());
    ThumbnailItem *item = m_saveJobs.value(job);
    qDebug() << item->pageId << "saved as" << hash;
    item->cid = hash;
    item->label = hash.left(12) + "\u2026";
    m_saveJobs.remove(job);
    if (m_saveJobs.isEmpty()) {
        qDebug() << "ready to save wrapper node";
        QJsonArray a;
        for (ThumbnailItem * item : items)
            a.append(item->cid);
        QJsonObject o;
        o.insert(QLatin1String("series"), a);
        QJsonDocument doc(o);
        qDebug() << doc.toJson();
        IpfsAgent agent;
        QJsonDocument result = agent.execPost("dag/put", "input-enc=json", doc);
        qDebug() << result.toJson();
        QString cid = result.object().value(QLatin1String("Cid")).toObject().value(QLatin1String("/")).toString();
        qDebug() << cid;
        emit seriesCidChanged(QUrl("ipfs:///" + cid));
    }
}
#endif
