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
//#include "common.h"

ThumbnailScene::ThumbnailScene() :
    cols(2), spacing(6), selectedIdx(-1),
    defaultItemIcon(":/32/document-new.png"),
    insertionPoint(),
    maxWidth(0), maxHeight(0)
{
    qDebug("ThumbnailScene constructor");
    setBackgroundBrush(QBrush(Qt::darkGray));
    QPen thickRed(Qt::red);
    thickRed.setWidth(4);
    insertionPoint.setPen(thickRed);
    QAction *newAction = m_contextMenu.addAction(tr("New"));
    connect(newAction, &QAction::triggered, this, &ThumbnailScene::appendBlank);
}

void ThumbnailScene::resize(int numItems)
{
    foreach(ThumbnailItem* item, items)
        delete item;
    items.resize(numItems);
    col = 0;
    row = 0;
    maxWidth = 0;
    maxHeight = 0;
    selectedIdx = -1;
}

void ThumbnailScene::clear()
{
    foreach(ThumbnailItem* item, items)
        delete item;
    items.clear();
}

void ThumbnailScene::add(int idx, QImage pm, QString label)
{
    ThumbnailItem* item = nullptr;
    //	qDebug("for idx %d got thumb with dims %d x %d", idx, pm.width(), pm.height());
    if (pm.width() < 1) {
        item = new ThumbnailItem(defaultItemIcon, idx, label);
    } else {
        QPixmap pxm = QPixmap::fromImage(pm);
        item = new ThumbnailItem(pxm, idx, label);
    }
    addItem(item);
    if (item->width() > maxWidth)
        maxWidth = item->width();
    if (item->height() > maxHeight)
        maxHeight = item->height();
    if (items.count() > idx && items[idx]) {
        removeItem(items[idx]);
        delete items[idx];
        update(sceneRect());
    }
    items[idx] = item;
    item->setPos(col++ * maxWidth + spacing, row * maxHeight + spacing);
    if (col >= cols) {
        col = 0;
        ++row;
    }
    if (selectedIdx < 0)
        selectedIdx = idx;
//	layout.addItem(item, idx, 0, Qt::AlignHCenter);
}

void ThumbnailScene::appendBlank()
{
    ThumbnailItem* item = new ThumbnailItem(defaultItemIcon, items.count(), tr("untitled"));
    addItem(item);
    if (item->width() > maxWidth)
        maxWidth = item->width();
    if (item->height() > maxHeight)
        maxHeight = item->height();
    items.append(item);
    item->setPos(col++ * maxWidth + spacing, row * maxHeight + spacing);
    if (col >= cols) {
        col = 0;
        ++row;
    }
    selectedIdx = items.count() - 1;
}

void ThumbnailScene::layout(int width, int height)
{
    col = 0;
    row = 0;
    if (width > 0 && maxWidth > 0)
        cols = (width + spacing) / (maxWidth + spacing);
    //qDebug("layout(%d), cols now %d, sceneRect %f x %f", width, cols, sceneRect().width(), sceneRect().height());
    int i = 0;
    foreach(ThumbnailItem* item, items)
    {
        if (col >= cols)
        {
            col = 0;
            ++row;
        }
        item->setPos(col++ * (maxWidth + spacing) + (maxWidth - item->width()) / 2,
                     row * (maxHeight + spacing) + (maxHeight - item->height()) / 2);
        //		item->selected = (i == selectedIdx ? true : false);
        ++i;
    }
    QRectF sceneBounds = sceneRect();
    if (height > 0)
        sceneBounds.setHeight(qMax(height, (row + 1) * (maxHeight + spacing)));
    if (width > 0)
        sceneBounds.setWidth(width);
    setSceneRect(sceneBounds);
}

void ThumbnailScene::currentPage(int idx)
{
    selectedIdx = idx;
    update(sceneRect());
    emit currentPageThumbnail(items[idx]);
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
    if (insIdx >= 0)
    {
        if (insIdx + 1 < items.size())
            emit statusMessage(QString("moving between %1 and %2").arg(items[insIdx]->pageIdx).arg(items[insIdx + 1]->pageIdx));
        else
            emit statusMessage(QString("moving to end"));
    }
    else
        emit statusMessage("moving to beginning");
}

void ThumbnailScene::dropEvent ( QGraphicsSceneDragDropEvent * ev )
{
    qDebug("dropEvent at %f, %f", ev->scenePos().x(), ev->scenePos().y());
    removeItem(&insertionPoint);
    emit statusClear();
    QPoint insColRow = insertionIdx(ev->scenePos());
    QList<QGraphicsItem *> sel = selectedItems();
    qSort(sel.begin(), sel.end(), lessThan);
    int insIdx = insColRow.y() * cols + insColRow.x() + 1;
    foreach (QGraphicsItem* item, sel)
    {
        int oldIdx = items.indexOf(static_cast<ThumbnailItem*>(item));
        if (oldIdx < 0)
            qWarning("item being dropped is not in the set of thumbnails!");
        else
            items.remove(oldIdx);
        if (oldIdx < insIdx)
            --insIdx;
    }
    foreach (QGraphicsItem* item, sel)
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
