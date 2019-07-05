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
#include "thumbnailitem.h"
#include "thumbnailscene.h"
#include <QWidget>
#include <QPainter>
#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QDebug>

ThumbnailItem::ThumbnailItem() :
    QGraphicsItem()
{
}

ThumbnailItem::ThumbnailItem(int pageNum, QString lbl) :
    QGraphicsItem(),
    pageId(pageNum),
	label(lbl)
{
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

QRectF ThumbnailItem::boundingRect() const
{
    return QRectF(0, 0, 128, 128);
}

void ThumbnailItem::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
    Q_UNUSED(option)
    Q_UNUSED(widget)
    painter->fillRect(boundingRect(), Qt::white);
    QFont font;
    font.setPointSize(6);
    painter->setFont(font);
    QRectF insetRect = boundingRect().adjusted(1, 1, -2, -2);
    painter->drawText(insetRect, Qt::TextWordWrap, content);
    if (static_cast<ThumbnailScene*>(scene())->currentPage() == pageId) {
        painter->setPen(Qt::red);
        painter->drawRect(insetRect);
    }
}

void ThumbnailItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton))
            .length() < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(event->widget());
    QMimeData *mime = new QMimeData;
    drag->setMimeData(mime);
    drag->exec();
}
