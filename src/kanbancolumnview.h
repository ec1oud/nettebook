/****************************************************************************
**
** Copyright (C) 2020 Shawn Rutledge
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

#ifndef KANBANCOLUMNVIEW_H
#define KANBANCOLUMNVIEW_H

#include <QAction>
#include <QMenu>
#include <QSplitter>
#include "document.h"
#include "textlistmodel.h"

class QTreeView;

class KanbanColumnView : public QSplitter
{
    Q_OBJECT

public:
    explicit KanbanColumnView(QWidget *parent = nullptr);
    ~KanbanColumnView();
    void setDocument(Document *doc);

protected:
    void onContextMenuRequested(QTreeView *view, TextListModel *model, const QPoint &pos);

private:
    Document *m_doc;
    QVector<TextListModel *> m_kanbanTrees;
    QTextList *m_doneList = nullptr;
    QMenu *m_itemContextMenu = new QMenu(this);
    QMenu *m_contextMenu = new QMenu(this);
    QAction *m_deleteAction = nullptr;
    QAction *m_addAction = nullptr;
};

#endif // KANBANCOLUMNVIEW_H
