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
    for (auto m : m_mimeDb.allMimeTypes())
        if (m.name() == QLatin1String("text/markdown")) {
            m_markdownType = m;
            break;
        }
}

QVariant Document::loadResource(int type, const QUrl &name)
{
//    qDebug() << static_cast<QTextDocument::ResourceType>(type) << name <<
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

void Document::loadUrl(QUrl url)
{
    QString urlString = url.toString();
    qDebug() << url << urlString;
    m_contentUrl = url;

    qDebug() << url << m_contentUrl << "baseUrl" << m_baseUrl << "relative?" << m_contentUrl.isRelative();
    int base58HashIndex = urlString.indexOf(base58HashPrefix);
    int base32HashIndex = urlString.indexOf(base32HashPrefix);
    if (base58HashIndex >= 0 || base32HashIndex >= 0)
        m_contentUrl.setScheme(ipfsScheme);
    else if (m_contentUrl.scheme().isEmpty())
        m_contentUrl.setScheme(fileScheme);
    if (m_contentUrl.isRelative() && base58HashIndex < 0 && base32HashIndex < 0) {
        QUrl res = m_contentUrl.resolved(baseUrl()); // doesn't work for local files
        qDebug() << url << "base" << baseUrl() << "resolved" << res << res.fileName() << m_contentUrl.toString();
        res.setScheme("file");
        if (res.fileName().isEmpty())
            res.setPath(res.path() + QLatin1Char('/') + url.fileName());
        qDebug() << url << res << res.fileName() << m_contentUrl.toString();
        m_contentUrl = res;
    }
    m_baseUrl = m_contentUrl.adjusted(QUrl::RemoveFilename);
    qDebug() << "URL for KIO:" << m_contentUrl << "baseURL for document:" << m_baseUrl;
    KIO::Job* job = KIO::get(m_contentUrl);
    connect (job, SIGNAL(data(KIO::Job *, const QByteArray &)),
             this, SLOT(dataReceived(KIO::Job *, const QByteArray &)));
    connect (job, SIGNAL(result(KJob*)), this, SLOT(dataReceiveDone(KJob*)));
}

/*!
    \return the last URL to be loaded via loadUrl()
*/
QUrl Document::contentUrl()
{
    return m_contentUrl;
}

void Document::dataReceived(KIO::Job *,const QByteArray & data )
{
//    qDebug() << "received" << data.size();
    m_rawText.append(data);
}

void Document::dataReceiveDone(KJob *)
{
    qDebug() << "received" << m_rawText.size();
    loadContent(m_rawText);
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

bool Document::loadContent(const QByteArray &content, QMimeType type)
{
    bool success = true;
    // Stupidly m_mimeDb.mimeTypeForData(content) can't recognize markdown.  With the filename it works.
    if (!type.isValid() || type.name() == QLatin1String("text/plain"))
        type = m_mimeDb.mimeTypeForFileNameAndData(m_contentUrl.fileName(), content);
    qDebug() << m_contentUrl.fileName() << "mime type" << type;
    setBaseUrl(m_baseUrl);
    if (type.name() == QLatin1String("text/markdown")) {
        setMarkdown(QString::fromUtf8(content));
    } else if (type.name() == QLatin1String("text/html") || type.name() == QLatin1String("application/xhtml+xml")) {
        QTextCodec *codec = Qt::codecForHtml(content);
        QString str = codec->toUnicode(content);
        setHtml(str);
    } else if (type.name() == QLatin1String("text/plain")) {
//        m_mainWidget->setCurrentFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        setPlainText(QString::fromLocal8Bit(content));
    }
    // TODO load images by writing a "loader" markdown file?
    else
        success = false;
    if (success)
        emit status(tr("Opened \"%1\"").arg(m_contentUrl.toString()));
    else
        emit status(tr("Could not open \"%1\"").arg(m_contentUrl.toString()));
    return success;
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
