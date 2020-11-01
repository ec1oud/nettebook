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

TextListModel::TextListModel(Document *doc, QTextBlock heading, QTextList *list, QObject *parent)
    : QAbstractListModel(parent), m_doc(doc), m_headingBlock(heading), m_list(list)
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

//bool DocumentTreeModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
//{
//    if (value != headerData(section, orientation, role)) {
//        // FIXME: Implement me!
//        emit headerDataChanged(orientation, section, section);
//        return true;
//    }
//    return false;
//}

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

//int TextListModel::columnCount(const QModelIndex &parent) const
//{
//    if (!parent.isValid())
//        return 0;

//    return 1;
//}

QVariant TextListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    switch (role) {
    case Qt::DisplayRole:
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
        // FIXME: Implement me!
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

Qt::ItemFlags TextListModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return QAbstractListModel::flags(index);

    return QAbstractListModel::flags(index) | Qt::ItemIsUserCheckable |
            Qt::ItemIsDragEnabled  | Qt::ItemIsDropEnabled |
            Qt::ItemNeverHasChildren | Qt::ItemIsEditable;
}

Qt::DropActions TextListModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

QStringList TextListModel::mimeTypes() const
{
    QStringList types;
    types << "application/vnd.text.list" << "text/plain" << "text/html" << "text/markdown";
    return types;
}

QMimeData *TextListModel::mimeData(const QModelIndexList &indices) const
{
    QMimeData *mimeData = new QMimeData;
    QByteArray encodedTextData;
    QDataStream textStream(&encodedTextData, QIODevice::WriteOnly);
    QTextDocument itemDocument;
    QTextCursor cursor(&itemDocument);

    qDebug() << "sending source list" << (quintptr)(m_list);
    textStream << (quintptr)(m_list);
    encodeData(indices, textStream);

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
    mimeData->setData("application/vnd.text.list", encodedTextData);
    return mimeData;
}

bool TextListModel::canDropMimeData(const QMimeData *data,
    Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(column);
    Q_UNUSED(parent);

    if (!data->hasFormat("application/vnd.text.list") &&
            !data->hasFormat("text/markdown") && !data->hasText())
        return false;

    return true;
}

bool TextListModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &destParent)
{
    if (!canDropMimeData(data, action, row, column, destParent))
        return false;
    qDebug() << action << row << column << destParent << data->text();
    if (data->hasFormat("application/vnd.text.list")) {
        row = destParent.row();
        if (row < 0 || row >= rowCount(destParent))
            row = rowCount(destParent) - 1;
        // decode and insert
        QByteArray encoded = data->data("application/vnd.text.list");
        QDataStream stream(&encoded, QIODevice::ReadOnly);

        QTextCursor cursor(m_doc);
        cursor.setPosition(m_list->item(row).position());
        cursor.movePosition(QTextCursor::EndOfBlock);
        qDebug() << "inserting after row" << row << "position" << cursor.position() << "text" << m_list->item(row).text();

        quintptr srcList;
        stream >> srcList;
        qDebug() << "receiving source list" << srcList;

        while (!stream.atEnd()) {
            int r, c;
            QMap<int, QVariant> v;
            stream >> r >> c >> v;
            beginInsertRows(destParent, row + 1, row + 1);
            cursor.insertBlock();
            cursor.insertText(v.value(Qt::DisplayRole).toString());
            auto fmt = cursor.blockFormat();
            fmt.setMarker(v.value(Qt::CheckStateRole) == Qt::Checked ?
                              QTextBlockFormat::MarkerType::Checked : QTextBlockFormat::MarkerType::Unchecked);
            cursor.setBlockFormat(fmt);
            m_list->add(cursor.block());
            endInsertRows();

            if (action == Qt::MoveAction) {
                // TODO is that really the receiver's responsibility? what if we drag from one nettebook process to another?
                QTextList *sourceList = (QTextList *)(srcList);
                qDebug() << "deleting row" << r << "of" << sourceList->count();
                QTextCursor srcCursor(sourceList->item(r));
                srcCursor.select(QTextCursor::BlockUnderCursor);
                sourceList->removeItem(r);
                srcCursor.removeSelectedText();
            }
        }
        beginResetModel();
        endResetModel();
    }
    return true;
}
