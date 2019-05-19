#include "document.h"
#include <QApplication>
#include <QDebug>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextCodec>
#include <KIO/Job>

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
//    qDebug() << static_cast<QTextDocument::ResourceType>(type) << name << url <<
//             (m_resourceLoaders.contains(name) ? "waiting for it" :
//             (m_loadedResources.contains(name) ? "from cache" : "new request"));
    if (m_resourceLoaders.contains(name))
        return QVariant(); // still waiting
    if (m_loadedResources.contains(name))
        return m_loadedResources.value(name);
    // not cached, so try to load it
    KIO::Job* job = KIO::get(name);
    connect (job, SIGNAL(data(KIO::Job *, const QByteArray &)),
             this, SLOT(resourceDataReceived(KIO::Job *, const QByteArray &)));
    connect (job, SIGNAL(result(KJob*)), this, SLOT(resourceReceiveDone(KJob*)));
    m_resourceLoaders.insert(name, job);
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

QJsonObject Document::filesList(QString url)
{
    // TODO load JSON file list via KIO
    /*
    ipfs::Json ls_result;
    m_ipfsClient.FilesLs(url.toLatin1().toStdString(), &ls_result);
    QByteArray json = QByteArray::fromStdString(ls_result.dump());
    std::cout << "FilesLs() result:" << std::endl << ls_result.dump(2) << std::endl;
    QJsonDocument doc = QJsonDocument::fromJson(json);
    return doc.object();
    */
    Q_UNUSED(url)
    return QJsonObject();
}

QByteArray Document::jsonDirectoryToMarkdown(QJsonObject j)
{
    QJsonArray links = j.value(QLatin1String("Links")).toArray();
    QByteArray ret;
    for (auto o : links) {
        auto object = o.toObject();
        auto hash = object.value(QLatin1String("Hash")).toString().toUtf8();
        auto name = object.value(QLatin1String("Name")).toString().toUtf8();
        ret += '[' + name + "](" + name + ") " + hash + "\n\n";
    }
    return ret;
}
