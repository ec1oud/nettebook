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
#ifndef THUMBNAILSCENE_H
#define THUMBNAILSCENE_H

#include <QGraphicsScene>
#include <QGraphicsGridLayout>
#include <QMenu>
#include "thumbnailitem.h"

class KJob;

/**
    \brief QGraphicsScene specialized for holding thumbnails for all of the
    pages in a series.

    Used in the thumbnails dock widget.

    Implements drag-and-drop page reordering.
*/
class ThumbnailScene : public QGraphicsScene
{
    Q_OBJECT

public:
    ThumbnailScene();

    int currentPage() { return selectedIdx; }
    QMenu * contextMenu() { return &m_contextMenu; }

public slots:
    void resize(int numItems);
    void add(int idx, QImage pm, QString label);
    void appendBlank();
    void layout(int width = -1, int height = -1);
    void currentPage(int idx);

    /**
        Override QGraphicsScene::clear() to also delete local stuff.
    */
    void clear();
    void saveAllToIpfs();

signals:
	void statusMessage(QString msg, int timeout = 0);
	void statusClear();
    void currentPageThumbnail(ThumbnailItem* it);
    void currentPageChanging(ThumbnailItem* it);
    void currentPageChanged(const QString &source, const QString &content);
    void seriesCidChanged(QUrl cid);

protected:
    void dragEnterEvent ( QGraphicsSceneDragDropEvent * ev );
    void dragMoveEvent ( QGraphicsSceneDragDropEvent * ev );
    void dropEvent ( QGraphicsSceneDragDropEvent * ev );

    /**
        At what index would the insertion occur *after*
        if dropped at the given position?
        E.g. if it's between items 5 and 6, will return 5.
     */
    QPoint insertionIdx(QPointF pos);

protected slots:
    void selectionChanged();
    void saveJobResult(KJob *job);

protected:
    int cols = 2;
    int spacing = 6;
    QVector<ThumbnailItem*> items;
    int selectedIdx = 0;
    QPixmap defaultItemIcon = QPixmap(":/128/empty-page.png");
    QGraphicsLineItem insertionPoint;
    QMenu m_contextMenu;
    int row = 0;
    int col = 0;
    int maxWidth = 0;
    int maxHeight = 0;

    QMap<KJob *,ThumbnailItem *> m_saveJobs;
};

#endif // THUMBNAILSCENE_H
