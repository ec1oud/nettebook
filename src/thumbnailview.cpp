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
#include "thumbnailview.h"
#include "thumbnailscene.h"
#include <QDebug>

ThumbnailView::ThumbnailView(QWidget* parent) :
    QGraphicsView(parent)
{
}

void ThumbnailView::resizeEvent(QResizeEvent *ev)
{
//qDebug("resizeEvent: %d x %d", ev->size().width(), ev->size().height());
    if (scene())
        static_cast<ThumbnailScene *>(scene())->layout(ev->size().width(), ev->size().height());
}

void ThumbnailView::contextMenuEvent(QContextMenuEvent *event)
{
    qDebug() << event->pos();
    static_cast<ThumbnailScene *>(scene())->contextMenu()->exec(mapToGlobal(event->pos()));
}
