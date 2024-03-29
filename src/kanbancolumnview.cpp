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

#include "kanbancolumnview.h"
#include "textlistmodel.h"
#include "settings.h"
#include <QTextList>
#include <QTreeView>

using namespace Qt::StringLiterals;

KanbanColumnView::KanbanColumnView(QWidget *parent) :
    QSplitter(parent)
{
    m_deleteAction = m_itemContextMenu->addAction(tr("Remove"));
    m_addAction = m_contextMenu->addAction(tr("Add"));
}

KanbanColumnView::~KanbanColumnView()
{
}

void KanbanColumnView::onContextMenuRequested(QTreeView *view, TextListModel *model, const QPoint &pos)
{
    QModelIndex idx = view->indexAt(pos);
    QAction *act = nullptr;
    if (idx.isValid())
        act = m_itemContextMenu->exec(view->mapToGlobal(pos));
    else
        act = m_contextMenu->exec(view->mapToGlobal(pos));
    qDebug() << view << pos << idx << "; selected" << act;
    if (act == m_deleteAction)
        model->removeRow(idx.row());
    else if (act == m_addAction) {
        idx = model->insertRowDefaultText(model->rowCount());
        view->edit(idx);
    }
}

void KanbanColumnView::setDocument(Document *doc)
{
    m_doc = doc;
    // iterate headings, match up with settings for the done column's name(s),
    // find lists under headings, make QTreeView columns as necessary
    Settings *settings = Settings::instance();
    QStringList doneHeadings = settings->stringOrDefault(settings->tasksGroup, settings->doneTasksHeadings)
            .split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    QTextCursor cursor(doc);
    QTextBlock heading;
    bool hasNext = true;
    bool isDoneList = false;
    bool foundDoneList = false;
    QTextList *list = nullptr;
    while (hasNext) {
        if (cursor.blockFormat().headingLevel()) {
            isDoneList = false;
            if (!list && heading.isValid())
                qDebug() << "apparently" << heading.text() << "is empty";
            heading = cursor.block();
            const QString headingText = heading.text();
            if (!foundDoneList) {
//                qDebug() << "comparing heading" << headingText << "to" << doneHeadings;
                for (const auto &expected : doneHeadings)
                    if (!headingText.compare(expected, Qt::CaseInsensitive)) {
                        isDoneList =  true;
                        qDebug() << "it's the 'done' list: matched one of" << doneHeadings;
                        break;
                    }
            }
            hasNext = cursor.movePosition(QTextCursor::NextBlock);
            list = nullptr;
        } else if (QTextList *curList = cursor.block().textList()) {
            if (!list) {
                list = curList;
                if (isDoneList)
                    m_doneList = list;
                TextListModel *model = new TextListModel(m_doc, heading, list,
                                                         isDoneList ? QTextBlockFormat::MarkerType::Checked :
                                                                      QTextBlockFormat::MarkerType::Unchecked,
                                                         this);
                m_kanbanTrees.append(model);
                qDebug() << "found" << heading.text() << "list with" << model->rowCount() << list->count() << "items";

                auto tree = new QTreeView(this);
                tree->setObjectName(heading.text() + "View"_L1);
                tree->setAcceptDrops(true);
                tree->setDragDropMode(QAbstractItemView::DragDrop);
                tree->setDefaultDropAction(Qt::MoveAction);
                tree->setSelectionMode(QAbstractItemView::ExtendedSelection);
                tree->setModel(model);
                tree->setContextMenuPolicy(Qt::CustomContextMenu);
                connect(tree, &QTreeView::customContextMenuRequested, tree,
                        [this, tree, model](const QPoint &pos) { onContextMenuRequested(tree, model, pos); });
                addWidget(tree); // add to QSplitter
            }
            // else: skip any nested lists within this one, for now
            // but eventually the model should be able to deal with nested lists
            while (hasNext && cursor.block().textList() == curList)
                hasNext = cursor.movePosition(QTextCursor::NextBlock);
        } else {
            hasNext = cursor.movePosition(QTextCursor::NextBlock);
        }
    }
}
