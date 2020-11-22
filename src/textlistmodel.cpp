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

#include "textlistmodel.h"

#include <QMimeData>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include <QTextList>

TextListModel::TextListModel(Document *doc, QTextBlock heading, QTextList *list,
                             QTextBlockFormat::MarkerType marker, QObject *parent)
    : QAbstractListModel(parent), m_doc(doc), m_headingBlock(heading), m_list(list), m_defaultMarker(marker)
{
}

QVariant TextListModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section);
    Q_UNUSED(orientation);
    switch (role) {
    case Qt::DisplayRole:
        return m_headingBlock.text();
        break;
    default:
        break;
    }
    return {};
}

//QModelIndex TextListModel::index(int row, int column, const QModelIndex &parent) const
//{
//    Q_UNUSED(parent)
//    return createIndex(row, column, nullptr);
//}

//QModelIndex TextListModel::parent(const QModelIndex &index) const
//{
//    Q_UNUSED(index);
//    return QModelIndex();
//}

int TextListModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return m_list->count();
}

QVariant TextListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    switch (role) {
    case Qt::ToolTipRole:
    case Qt::DisplayRole:
    case Qt::EditRole:
        return m_list->item(index.row()).text();
        break;
    case Qt::CheckStateRole:
        return (m_list->item(index.row()).blockFormat().marker() == QTextBlockFormat::MarkerType::Checked ?
                    Qt::Checked : Qt::Unchecked);
        break;
    default:
        break;
    }

    return {};
}

bool TextListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
qDebug() << index << role << value;
    if (data(index, role) != value) {

        QTextCursor cursor(m_doc);
        cursor.setPosition(m_list->item(index.row()).position());
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        cursor.insertText(value.toString());
        emit dataChanged(index, index, QVector<int>() << role);
        return true;
    }
    return false;
}

bool TextListModel::insertRows(int row, int count, const QModelIndex &parent)
{
    qDebug() << row << count << parent;
    if (row >= m_list->count())
        row = m_list->count() - 1;
    const int itemPos = m_list->item(row).position();
    QTextCursor cursor(m_doc);
    cursor.setPosition(itemPos);
    cursor.movePosition(QTextCursor::EndOfBlock);
    cursor.insertBlock();
    m_list->add(cursor.block());
    return true;
}

bool TextListModel::removeRows(int row, int count, const QModelIndex &parent)
{
    qDebug() << row << count << parent;
    beginRemoveRows(parent, row, row + count - 1);
    for (int r = row; r < row + count; ++r) {
        QTextCursor cursor(m_list->item(row));
        cursor.select(QTextCursor::BlockUnderCursor);
        m_list->removeItem(row);
        cursor.removeSelectedText();
    }
    endRemoveRows();
    return true;
}

QModelIndex TextListModel::insertRowDefaultText(int row)
{
    beginInsertRows(QModelIndex(), row, row);
    insertRow(row);
    auto idx = index(row);
    setData(idx, tr("new task"));
    endInsertRows();
    return idx;
}

Qt::ItemFlags TextListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractListModel::flags(index) | Qt::ItemIsDropEnabled;

    return QAbstractListModel::flags(index) | Qt::ItemIsUserCheckable |
            Qt::ItemIsDragEnabled | Qt::ItemNeverHasChildren | Qt::ItemIsEditable;
}

Qt::DropActions TextListModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList TextListModel::mimeTypes() const
{
    QStringList types;
    types << "text/plain" << "text/html" << "text/markdown";
    return types;
}

QMimeData *TextListModel::mimeData(const QModelIndexList &indices) const
{
    QMimeData *mimeData = new QMimeData;
    QTextDocument itemDocument;
    QTextCursor cursor(&itemDocument);

    for (const QModelIndex &index : indices) {
        if (index.isValid()) {
            QTextCursor sourceCursor(m_doc);
            sourceCursor.setPosition(m_list->item(index.row()).position());
            sourceCursor.select(QTextCursor::BlockUnderCursor);
            cursor.insertFragment(QTextDocumentFragment(sourceCursor));
        }
    }

    mimeData->setText(itemDocument.toPlainText());
    mimeData->setHtml(itemDocument.toHtml());
    mimeData->setData("text/markdown", itemDocument.toMarkdown().toUtf8());
    return mimeData;
}

bool TextListModel::canDropMimeData(const QMimeData *data,
    Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);

    if (!data->hasFormat("text/markdown") && !data->hasText() && !data->hasHtml())
        return false;

//    qDebug() << "OK for" << action << row << "parent" << parent;
    return true;
}

bool TextListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &destParent)
{
    if (!canDropMimeData(data, action, row, column, destParent))
        return false;
    qDebug() << action << row << column << destParent << data->text();

    bool expectListItems = true;
    QTextDocument tmpDoc;
    if (data->hasFormat("text/markdown")) {
        tmpDoc.setMarkdown(QString::fromUtf8(data->data(QLatin1String("text/markdown"))));
    } else if (data->hasHtml()) {
        tmpDoc.setHtml(data->html());
    } else if (data->hasText()) {
        tmpDoc.setPlainText(data->text());
        expectListItems = false;
    }

    // decode and insert
    QTextCursor cursor(m_doc);
    if (Q_UNLIKELY(row >= rowCount(destParent)))
        row = rowCount(destParent) - 1;
    if (row < 0) {
        cursor.setPosition(m_list->item(rowCount(destParent) - 1).position());
        cursor.movePosition(QTextCursor::EndOfBlock);
    } else {
        cursor.setPosition(m_list->item(row).position());
    }
    qDebug() << "inserting before row" << row << "position" << cursor.position() << "text" << m_list->item(row).text();

    int rowCount = 0;
    QTextCursor tmpCursor(&tmpDoc);
    QTextBlockFormat fmt;
    fmt.setMarker(m_defaultMarker);
    bool hasNext = true;
    // copy text of each task
    while (hasNext) {
        if (tmpCursor.block().textList() || !expectListItems) {
            ++rowCount;
            beginInsertRows(destParent, row + rowCount, row + rowCount);
            QString text = tmpCursor.block().text().trimmed();
            // TODO Qt needs QTextCursor::insertMarkdown() so that we can avoid losing formatting
            if (row < 0)
                cursor.insertText(QLatin1String("\n") + text);
            else
                cursor.insertText(text + QLatin1Char('\n'));
            cursor.setBlockFormat(fmt);
            m_list->add(cursor.block());
            endInsertRows();
        }
        hasNext = tmpCursor.movePosition(QTextCursor::NextBlock);
    }

    return true;
}
