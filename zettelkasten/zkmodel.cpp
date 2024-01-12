// Copyright (C) 2024 Shawn Rutledge <s@ecloud.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "zkmodel.h"

#include <QLoggingCategory>
#include <QQuickTextDocument>
#include <QRegularExpression>
#include <QTextFragment>

Q_LOGGING_CATEGORY(lcZkm, "org.nettebook.zettelkasten.model")

static QRegularExpression SpaceReplacementRE("[_-]");
static QRegularExpression SpaceRE("\\s+");

ZkModel::ZkModel(QObject *parent)
    : QAbstractTableModel(parent) //, m_roles(QAbstractTableModel::roleNames())
{
    // m_roles.insert(int(Role::Url), QByteArrayLiteral("fileUrl"));
    // m_roles.insert(int(Role::FileLastModified), QByteArrayLiteral("fileModified"));
    qCDebug(lcZkm) << roleNames();

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
        { int(Role::LinkedIndices), "linkedIndices" },
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

    // qCDebug(lcZkm) << index << "role" << roleNames()[role] << fi << QUrl::fromLocalFile(m_dir.filePath(fi.fileName())) << fi.lastModified();

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
    case int(Role::LinkedIndices):
        // return getLinkedIndices(index.row()); // how to get it into QVariant? QStringList?
        return {};
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
    qCDebug(lcZkm) << Q_FUNC_INFO << QDir::currentPath() << newFolder << newFolder.toLocalFile() << m_dir.absolutePath();
    qCDebug(lcZkm) << "found" << rowCount();
    emit folderChanged();
}

bool ZkModel::watcherEnabled() const
{
    return m_watcherEnabled;
}

void ZkModel::setWatcherEnabled(bool newWatcherEnabled)
{
    if (m_watcherEnabled == newWatcherEnabled)
        return;
    m_watcherEnabled = newWatcherEnabled;
    emit watcherEnabledChanged();
    if (m_watcherMissedReset) {
        m_watcherMissedReset = false;
        beginResetModel();
        endResetModel();
    }
}

void ZkModel::deleteFile(const QUrl &filename)
{
    QFileInfo fi(filename.toLocalFile());
    if (!fi.exists()) {
        qWarning() << "can't find" << fi;
        return;
    }
    QFile f(fi.absoluteFilePath());
    f.remove();
}

void ZkModel::rename(const QUrl &filename, const QString &newTitle)
{
    QString newFilename = newTitle.trimmed();
    newFilename.replace(SpaceRE, "-"); // kebab case
    QFileInfo old(filename.toLocalFile());
    if (!old.exists()) {
        qWarning() << "can't find" << old;
        return;
    }
    newFilename += "." + old.suffix();
    QFileInfo fi(m_dir, newFilename);
    qCDebug(lcZkm) << "rename" << old.absoluteFilePath() << fi.absoluteFilePath();
    QFile::rename(old.absoluteFilePath(), fi.absoluteFilePath());
}

QString ZkModel::makeNew()
{
    QDateTime now = QDateTime::currentDateTime();
    QString name = now.toString(Qt::ISODateWithMs) + ".md";
    QFile f(m_dir.filePath(name));
    qCDebug(lcZkm) << ">>> new" << f.fileName();
    f.open(QIODevice::WriteOnly | QIODevice::NewOnly); // touch it
    f.close();
    return f.fileName();
}

QQuickTextDocument *ZkModel::getDocument(int row)
{
    if (m_documentProvider.isCallable()) {
        auto const args = QJSValueList() << QJSValue(row);
        auto doc = m_documentProvider.call(args).toQObject();
        return qobject_cast<QQuickTextDocument *>(doc);
    }
    return nullptr;
}

QList<int> ZkModel::getLinkedIndices(int row) const
{
    QList<int> ret;
    QStringList files = m_dir.entryList(QDir::Files);
    auto doc = const_cast<ZkModel *>(this)->getDocument(row);
    if (!doc || !doc->textDocument()) {
        qWarning() << "no document for row" << row << files.at(row);
        return ret;
    }
    QTextDocument *qtd = doc->textDocument();
    // qCDebug(lcZkm) << "row" << row << "has doc" << doc->source() << qtd;
    // QTextCursor cur(qtd);
    auto firstBlock = qtd->firstBlock();
    for (QTextBlock::iterator it = firstBlock.begin(); !(it.atEnd()); ++it) {
        QTextFragment currentFragment = it.fragment();
        QTextCharFormat fmt = currentFragment.charFormat();
        if (fmt.hasProperty(QTextFormat::AnchorHref)) {
            QUrl ref(fmt.anchorHref());
            if (ref.scheme().isEmpty() || ref.isLocalFile()) {
                int index = files.indexOf(ref.fileName());
                qCDebug(lcZkm) << "    found local link" << ref << ref.fileName() << fmt.anchorNames() << "@" << index
                         << "on" << currentFragment.text();
                if (index >= 0)
                    ret << index;
            }
        }
    }
    return ret;
}

QJSValue ZkModel::documentProvider() const
{
    return m_documentProvider;
}

void ZkModel::setDocumentProvider(const QJSValue &newDocumentProvider)
{
    if (newDocumentProvider.strictlyEquals(m_documentProvider))
        return;
    m_documentProvider = newDocumentProvider;
    emit documentProviderChanged();
}

void ZkModel::onFileChanged(const QString &path)
{
    qCDebug(lcZkm) << "fileChanged" << path;
    // TODO look up which row that applies to and emit dataChanged()
}

void ZkModel::onDirectoryChanged(const QString &path)
{
    qCDebug(lcZkm) << "directoryChanged" << path;
    if (m_watcherEnabled) {
        beginResetModel();
        endResetModel();
    } else {
        m_watcherMissedReset = true;
    }
}

#include "moc_zkmodel.cpp"
