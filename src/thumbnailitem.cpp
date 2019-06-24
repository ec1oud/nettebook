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
    QGraphicsPixmapItem()
{
}

ThumbnailItem::ThumbnailItem(const QPixmap& pm, int pageNum, QString lbl) :
	QGraphicsPixmapItem(pm),
//	selected(false),
    pageId(pageNum),
	label(lbl)
{
//	qDebug("ThumbnailItem(pixmap dims %d x %d, page %d: %s)",
//			pm.width(), pm.height(), pageNum, lbl.toAscii().constData());
    setFlag(QGraphicsItem::ItemIsMovable, true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
}

void ThumbnailItem::paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
//	if (widget)
//		qDebug("paint into widget: %s", widget->metaObject()->className() );
    QGraphicsPixmapItem::paint(painter, option, widget);
//	if (selected)
//	{
//		painter->setPen(QPen(Qt::darkGreen, 3));
//		painter->drawRect(pixmap().rect().adjusted(-4, -4, 4, 4));
//	}
	QString pageStr = label;
	if (pageStr.isEmpty())
        pageStr = QString::number(pageId);
	QRect pageBounds = painter->boundingRect(0, 0, width(), height() - 1, Qt::AlignHCenter| Qt::AlignBottom, pageStr);
	QPainterPath pp;
	pp.moveTo(pageBounds.bottomRight());
	pp.arcTo(pageBounds.topRight().x(), pageBounds.topRight().y(), pageBounds.height() / 2, pageBounds.height() - 1, 270, 180);
//	pp.lineTo(pageBounds.topLeft());
	pp.arcTo(pageBounds.topLeft().x(), pageBounds.topLeft().y(), pageBounds.height() / -2, pageBounds.height() - 1, 90, -180);
	pp.lineTo(pageBounds.bottomRight());
	pp.closeSubpath();
	QColor pageLabelFill(Qt::lightGray);
    if (((ThumbnailScene*)scene())->currentPage() == pageId)
		pageLabelFill = Qt::red;
	pageLabelFill.setAlpha(220);
	painter->fillPath(pp, QBrush(pageLabelFill));
	painter->drawText(pageBounds, Qt::AlignCenter, pageStr);
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
