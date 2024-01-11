// Copyright (C) 2024 Shawn Rutledge <s@ecloud.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZKMODEL_H
#define ZKMODEL_H

#include <QAbstractTableModel>
#include <QDir>
#include <QFileSystemWatcher>
#include <QQmlEngine>

class ZkModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(QUrl folder READ folder WRITE setFolder NOTIFY folderChanged FINAL)
    QML_ELEMENT

public:
    enum class Role : int {
        Title = Qt::DisplayRole,
        FilePath = Qt::UserRole,
        FileName,
        FileLastModified,
        Url,
        Links
    };

    explicit ZkModel(QObject *parent = nullptr);

    QHash<int, QByteArray> roleNames() const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    // QVariant headerData(int section,
    //                     Qt::Orientation orientation,
    //                     int role = Qt::DisplayRole) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QUrl folder() const;
    void setFolder(const QUrl &newFolder);

    Q_INVOKABLE void rename(const QUrl &filename, const QString &newTitle);

signals:
    void folderChanged();

private:
    void onDirectoryChanged(const QString &path);
    void onFileChanged(const QString &path);

private:
    // QHash<int, QByteArray> m_roles;
    QUrl m_folder;
    QDir m_dir;
    QFileSystemWatcher m_watcher;
};

#endif // ZKMODEL_H
