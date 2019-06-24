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
#ifndef THUMBNAILVIEW_H
#define THUMBNAILVIEW_H

class ThumbnailItem;

#include <QGraphicsView>
#include <QMenu>

/**
    \brief QGraphicsView specialized for holding a ThumbnailScene and
    triggering layout when it is resized.
*/
class ThumbnailView : public QGraphicsView
{
    Q_OBJECT
public:
    ThumbnailView(QWidget* parent = nullptr);

    void setContextMenu(QMenu *menu) { m_contextMenu = menu; }

public slots:
    void ensureChildVisible(QGraphicsItem* item) { ensureVisible(item); }

protected:
    void resizeEvent (QResizeEvent * ev) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

private:
    QMenu *m_contextMenu = nullptr;
    QAction *m_newAction;
};

#endif // THUMBNAILVIEW_H
