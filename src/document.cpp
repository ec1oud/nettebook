#include "document.h"
#include <QApplication>
#include <QDebug>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextCodec>
#include <KIO/Job>
#include <KIO/ListJob>

static const QString ipfsScheme = QStringLiteral("ipfs");
static const QString fileScheme = QStringLiteral("file");
static const QString base58HashPrefix = QStringLiteral("Qm");
static const QString base32HashPrefix = QStringLiteral("bafybei");

Document::Document(QObject *parent) : QTextDocument(parent)
{
}

QVariant Document::loadResource(int type, const QUrl &name)
{
    Q_UNUSED(type) // we always return QByteArray and the caller knows what to do (?)
    QUrl url(name);
    QString urlString = url.toString();
    int base58HashIndex = urlString.indexOf(base58HashPrefix);
    int base32HashIndex = urlString.indexOf(base32HashPrefix);
    if (base58HashIndex >= 0 || base32HashIndex >= 0)
        url.setScheme(ipfsScheme);
    if (url.isRelative() && base58HashIndex < 0 && base32HashIndex < 0) {
        qDebug() << "resolving relative URL" << url << "base" << baseUrl() << "meta" << metaInformation(DocumentUrl);
        url = baseUrl().resolved(url);
    }
    if (m_resourceLoaders.contains(name))
        return QVariant(); // still waiting
    else
        qDebug() << static_cast<QTextDocument::ResourceType>(type) << name << url <<
                    (m_loadedResources.contains(name) ? "from cache" : "new request");
    if (url.fileName().isEmpty()) {
        if (!m_fileList.isEmpty()) {
            QByteArray ret = fileListMarkdown();
            m_fileList.clear();
            return ret;
        } else if (!m_errorText.isEmpty()) {
            QString ret = m_errorText;
            m_errorText.clear();
            return ret;
        }
        m_fileList.clear();
        KIO::ListJob* job = KIO::listDir(url);
        connect (job, SIGNAL(entries(KIO::Job*, const KIO::UDSEntryList &)),
                 this, SLOT(fileListReceived(KIO::Job*, const KIO::UDSEntryList &)));
        m_resourceLoaders.insert(name, job);
    } else {
        if (m_loadedResources.contains(name))
            return m_loadedResources.value(name);
        // not cached, so try to load it
        KIO::Job* job = KIO::get(name);
qDebug() << "GET" << name << job;
        connect (job, SIGNAL(data(KIO::Job *, const QByteArray &)),
                 this, SLOT(resourceDataReceived(KIO::Job *, const QByteArray &)));
        connect (job, SIGNAL(result(KJob*)), this, SLOT(resourceReceiveDone(KJob*)));
        m_resourceLoaders.insert(name, job);
    }
    // Waiting for now, but this function can't block.  We'll be nagged again soon.
    return QVariant();
}

void Document::resourceDataReceived(KIO::Job *job, const QByteArray & data)
{
    QUrl url = m_resourceLoaders.key(job);
    // TODO check data for error messages
//qDebug() << job << "received" << data.size() << url;
    if (m_loadedResources.contains(url))
        m_loadedResources[url].append(data);
    else
        m_loadedResources.insert(url, data);
}

void Document::resourceReceiveDone(KJob *job)
{
    QUrl url = m_resourceLoaders.key(job);
    qDebug() << "for" << url.toString() << "got" << m_loadedResources.value(url).size() << "bytes";
    if (!m_loadedResources.value(url).size())
        m_loadedResources[url].append(tr("%1: empty document").arg(url.toString()).toUtf8());
    m_resourceLoaders.remove(url);
    emit documentLayoutChanged();
    if (m_resourceLoaders.isEmpty()) {
        emit allResourcesLoaded();
        qDebug() << "all resources loaded";
    }
}

void Document::fileListReceived(KIO::Job *job, const KIO::UDSEntryList &list)
{
    QUrl url = m_resourceLoaders.key(job);
    if (job->error()) {
        m_errorText = job->errorString();
        m_fileList.clear();
    } else {
        m_fileList = list;
        if (list.count() == 0)
            m_errorText = tr("no files in %1").arg(url.toString());
        qDebug() << "got file list" << list.count() << m_fileList;
    }
    m_resourceLoaders.remove(url);
}

QByteArray Document::fileListMarkdown()
{
    QByteArray ret = "|File|Hash|Size|\n"
                     "|----|----|----|\n";
    for (const KIO::UDSEntry &f : m_fileList) {
        auto name = f.stringValue(KIO::UDSEntry::UDS_NAME);
        if (f.isDir())
            name += "/";
        auto hash = f.stringValue(KIO::UDSEntry::UDS_LINK_DEST);
        qint64 size = f.numberValue(KIO::UDSEntry::UDS_SIZE);
        ret += "|[" + name + "](" + name + ")|" + hash +
                "|" + QLocale().formattedDataSize(size) +
//                "|" + f.stringValue(KIO::UDSEntry::UDS_CREATION_TIME) +
//                "|" + f.stringValue(KIO::UDSEntry::UDS_MIME_TYPE) +
                "|\n";
    }
    qDebug() << ret;
    return ret;
}
