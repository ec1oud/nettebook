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
#ifndef THUMBNAILITEM_H
#define THUMBNAILITEM_H

#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>

/**
    \brief A thumbnail to be shown in the QGraphicsView in the thumbnails dock widget.
*/
class ThumbnailItem : public QGraphicsItem
{
public:
    ThumbnailItem();
    ThumbnailItem(int pageNum, QString lbl);


    int pageId = 0; // not really page number, just a unique and initially sequential ID
    QString label;

    // TODO stop abusing this as a model
    QString cid;
    QString content;
    bool saved = false;

    QRectF boundingRect() const override;
    void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget = nullptr ) override;

protected:
    void mouseMoveEvent ( QGraphicsSceneMouseEvent * ev ) override;
};

#endif // THUMBNAILITEM_H
