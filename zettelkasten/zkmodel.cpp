// Copyright (C) 2024 Shawn Rutledge <s@ecloud.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "zkmodel.h"

#include <QRegularExpression>

static QRegularExpression SpaceReplacementRE("[_-]");

ZkModel::ZkModel(QObject *parent)
    : QAbstractTableModel(parent) //, m_roles(QAbstractTableModel::roleNames())
{
    // m_roles.insert(int(Role::Url), QByteArrayLiteral("fileUrl"));
    // m_roles.insert(int(Role::FileLastModified), QByteArrayLiteral("fileModified"));
    qDebug() << roleNames();

    connect(&m_watcher, &QFileSystemWatcher::directoryChanged,
            this, &ZkModel::onDirectoryChanged);
    connect(&m_watcher, &QFileSystemWatcher::fileChanged,
            this, &ZkModel::onFileChanged);
}

QHash<int, QByteArray> ZkModel::roleNames() const
{
    static const QHash<int, QByteArray> roles {
        { int(Role::Title), "title" },
        { int(Role::FilePath), "filePath" },
        { int(Role::FileName), "fileName" },
        { int(Role::FileLastModified), "fileModified" },
        { int(Role::Url), "fileUrl" },
        { int(Role::Links), "links" },
    };
    return roles;
}

int ZkModel::rowCount(const QModelIndex &) const
{
    return m_dir.entryInfoList(QDir::Files).size();
}

int ZkModel::columnCount(const QModelIndex &) const
{
    return 1;
}

// QVariant ZkModel::headerData(int section, Qt::Orientation orientation, int role) const
// {
//     return QString("header");
// }

QVariant ZkModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    QFileInfo fi = m_dir.entryInfoList(QDir::Files).at(index.row());

    qDebug() << index << "role" << roleNames()[role] << fi << QUrl::fromLocalFile(m_dir.filePath(fi.fileName())) << fi.lastModified();

    switch (role) {
    case int(Role::Url):
        return QUrl::fromLocalFile(m_dir.filePath(fi.fileName()));
    case int(Role::FileName):
        return fi.fileName();
    case int(Role::FileLastModified): {
        return fi.lastModified();
    }
    case int(Role::Title):
        return fi.baseName().replace(SpaceReplacementRE, " ");
    }

    return QVariant();
}

QUrl ZkModel::folder() const
{
    return m_folder;
}

void ZkModel::setFolder(const QUrl &newFolder)
{
    if (m_folder == newFolder)
        return;
    m_folder = newFolder;

    m_dir = QDir(newFolder.toLocalFile());
    m_watcher.addPath(m_dir.absolutePath());
    qDebug() << Q_FUNC_INFO << QDir::currentPath() << newFolder << newFolder.toLocalFile() << m_dir.absolutePath();
    qDebug() << "found" << rowCount();
    emit folderChanged();
}

void ZkModel::onFileChanged(const QString &path)
{
    qDebug() << "fileChanged" << path;
    // TODO look up which row that applies to and emit dataChanged()
}

void ZkModel::onDirectoryChanged(const QString &path)
{
    qDebug() << "directoryChanged" << path;
    beginResetModel();
    endResetModel();
}


#include "moc_zkmodel.cpp"
