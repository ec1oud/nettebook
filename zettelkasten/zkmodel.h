// Copyright (C) 2024 Shawn Rutledge <s@ecloud.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ZKMODEL_H
#define ZKMODEL_H

#include <QAbstractTableModel>
#include <QDir>
#include <QFileSystemWatcher>
#include <QQmlEngine>

class QQuickTextDocument;

class ZkModel : public QAbstractTableModel
{
    Q_OBJECT
    Q_PROPERTY(QUrl folder READ folder WRITE setFolder NOTIFY folderChanged FINAL)
    Q_PROPERTY(QJSValue documentProvider READ documentProvider WRITE setDocumentProvider NOTIFY documentProviderChanged FINAL)
    Q_PROPERTY(bool watcherEnabled READ watcherEnabled WRITE setWatcherEnabled NOTIFY watcherEnabledChanged FINAL)
    QML_ELEMENT

public:
    enum class Role : int {
        Title = Qt::DisplayRole,
        FilePath = Qt::UserRole,
        FileName,
        FileLastModified,
        Url,
        LinkedIndices
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

    QJSValue documentProvider() const;
    void setDocumentProvider(const QJSValue &newDocumentProvider);

    Q_INVOKABLE void rename(const QUrl &filename, const QString &newTitle);
    Q_INVOKABLE QString makeNew();
    Q_INVOKABLE QList<int> getLinkedIndices(int row) const;

    bool watcherEnabled() const;
    void setWatcherEnabled(bool newWatcherEnabled);

signals:
    void folderChanged();
    void documentProviderChanged();

    void watcherEnabledChanged();

private:
    void onDirectoryChanged(const QString &path);
    void onFileChanged(const QString &path);
    QQuickTextDocument *getDocument(int row);

private:
    // QHash<int, QByteArray> m_roles;
    QUrl m_folder;
    QDir m_dir;
    QFileSystemWatcher m_watcher;
    QJSValue m_documentProvider;
    bool m_watcherEnabled = true;
    bool m_watcherMissedReset = false;
};

#endif // ZKMODEL_H
