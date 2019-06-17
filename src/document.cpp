#include "document.h"
#include "cidfinder.h"
#include <QApplication>
#include <QBuffer>
#include <QDebug>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextCodec>
#include <QTextDocumentWriter>
#include <KIO/Job>
#include <KIO/ListJob>

static const QString ipfsScheme = QStringLiteral("ipfs");
static const QString fileScheme = QStringLiteral("file");

Document::Document(QObject *parent) : QTextDocument(parent)
{
}

QVariant Document::loadResource(int t, const QUrl &name)
{
    QTextDocument::ResourceType type = static_cast<QTextDocument::ResourceType>(t);
    if (m_status >= NullStatus && (type == ResourceType::HtmlResource || type == ResourceType::MarkdownResource))
        setStatus(LoadingMain);
    QUrl url(name);
    QString urlString = url.toString();
    CidFinder::Result cidResult = CidFinder::findIn(urlString);
    if (cidResult.isValid())
        url.setScheme(ipfsScheme);
    else if (url.isRelative()) {
        qDebug() << "resolving relative URL" << url << "base" << baseUrl() << "meta" << metaInformation(DocumentUrl);
        url = baseUrl().resolved(url);
    }
    if (m_resourceLoaders.contains(name))
        return QVariant(); // still waiting
    else
        qDebug() << type << name << url <<
                    (m_loadedResources.contains(name) ? "from cache" : "new request");
    if (url.fileName().isEmpty()) {
        if (!m_fileList.isEmpty()) {
            QByteArray ret = fileListMarkdown();
            m_fileList.clear();
            setStatus(Ready);
            return ret;
        } else if (!m_errorText.isEmpty()) {
            QString ret = m_errorText;
            m_errorText.clear();
            setStatus(ErrorWithText);
            return ret;
        }
        m_fileList.clear();
        KIO::ListJob* job = KIO::listDir(url);
        connect (job, SIGNAL(entries(KIO::Job*, const KIO::UDSEntryList &)),
                 this, SLOT(fileListReceived(KIO::Job*, const KIO::UDSEntryList &)));
        m_resourceLoaders.insert(name, job);
    } else {
        bool main = false;
        if (type == ResourceType::HtmlResource || type == ResourceType::MarkdownResource) {
            m_mainFile = url;
            m_saveType = static_cast<QTextDocument::ResourceType>(type);
            main = true;
        }
        if (m_loadedResources.contains(name)) {
            if (m_status < NullStatus) {
                if (main && m_mainFile.scheme().isEmpty()) {
                    m_mainFile = m_mainFile.fromLocalFile(url.path());
                    emit contentSourceChanged(m_mainFile);
                    qDebug() << "assuming missing local file" << m_mainFile;
                }
            }
            emit resourceLoaded(name);
            return m_loadedResources.value(name);
        }
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

void Document::setStatus(Document::Status s)
{
    m_status = s;
    // TODO emit?
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
    if (!m_loadedResources.value(url).size()) {
        if (m_status == LoadingMain)
            setStatus(ErrorEmpty);
        m_loadedResources[url].append(tr("%1: empty document").arg(url.toString()).toUtf8());
    } else {
        QMimeType type = QMimeDatabase().mimeTypeForFileNameAndData(url.fileName(), m_loadedResources[url]);
        qDebug() << "detected mime type" << type.name();
        if (type.name().contains(QLatin1String("html")))
            m_saveType = HtmlResource;
        if (m_status == LoadingMain)
            setStatus(Ready);
        // not useful because Qt doesn't have an OpenDoc reader yet
//        else if (type.name().contains(QLatin1String("opendocument")))
//            m_saveType = OdtResource;
        // we can't fully trust the markdown/plain text detection
    }
    m_resourceLoaders.remove(url);
    emit documentLayoutChanged();
    emit resourceLoaded(url);
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
        if (list.count() == 0) {
            m_errorText = tr("no files in %1").arg(url.toString());
            setStatus(ErrorEmpty);
        } else {
            setStatus(Ready);
        }
        qDebug() << "got file list" << list.count() << m_fileList;
    }
    m_resourceLoaders.remove(url);
    emit resourceLoaded(url);
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

void Document::saveAs(const QUrl &url, const QString &mimeType)
{
    m_saveDone = false;
    qDebug() << url << mimeType;
    QString mt = mimeType;
    if (mimeType.contains(QLatin1String("opendocument")))
        m_saveType = OdtResource;
    else if (mimeType == QLatin1String("text/plain"))
        m_saveType = PlainTextResource;
    else if (m_saveType == ResourceType::HtmlResource)
        mt = QLatin1String("application/xhtml+xml");
    else if (m_saveType == ResourceType::MarkdownResource)
        mt = QLatin1String("text/markdown");
    KIO::TransferJob *job = KIO::put(url, -1, KIO::Overwrite);
    job->addMetaData("content-type", mt);
    connect (job, SIGNAL(dataReq(KIO::Job *, QByteArray &)),
             this, SLOT(onSaveDataReq(KIO::Job *, QByteArray &)));
    connect (job, SIGNAL(result(KJob*)), this, SLOT(onSaveDone(KJob*)));
}

/*!
    Save to a new file, and emit saved() to provide the hash when done.
*/
void Document::saveToIpfs()
{
    m_saveDone = false;
    QString mt;
    switch (m_saveType) {
    case HtmlResource:
        mt = QLatin1String("application/xhtml+xml");
        break;
    case PlainTextResource:
        mt = QLatin1String("text/plain");
        break;
    default:
        mt = QLatin1String("text/markdown");
        m_saveType = ResourceType::MarkdownResource;
        break;
    }
    QUrl url("ipfs:///");
    m_transferJob = KIO::put(url, -1, KIO::Overwrite);
    m_transferJob->addMetaData("content-type", mt);
    connect (m_transferJob, SIGNAL(dataReq(KIO::Job *, QByteArray &)),
             this, SLOT(onSaveDataReq(KIO::Job *, QByteArray &)));
    connect (m_transferJob, SIGNAL(result(KJob*)), this, SLOT(onSaveDone(KJob*)));
}

void Document::onSaveDataReq(KIO::Job *job, QByteArray &dest)
{
    Q_UNUSED(job)
    if (m_saveDone)
        return;
    switch (m_saveType) {
    case ResourceType::HtmlResource:
        qDebug() << "writing HTML";
        dest = toHtml().toLocal8Bit();
        break;
    case ResourceType::MarkdownResource:
        qDebug() << "writing Markdown";
        dest = toMarkdown().toUtf8();
        break;
    case PlainTextResource:
        qDebug() << "writing plain text";
        dest = toPlainText().toLocal8Bit();
        break;
    case OdtResource: {
        qDebug() << "writing ODT";
        QBuffer buf(&dest);
        QTextDocumentWriter writer(&buf, "odt");
        writer.write(this);
    } break;
    default:
        qWarning() << "saving" << m_saveType << "isn't implemented";
        break;
    }
    m_saveDone = true;
}

void Document::onSaveDone(KJob *job)
{
    Q_UNUSED(job)
    if (m_transferJob) {
        QString hash = m_transferJob->metaData().value(QLatin1String("Hash"), QString());
        qDebug() << m_transferJob->metaData();
        if (!hash.isEmpty()) {
            QUrl url(QLatin1String("ipfs:///") + hash);
            setStatus(Ready);
            emit saved(url);
        } else {
            m_errorText = QLatin1String("save failed: no hash");
            setStatus(ErrorWithText);
        }
    } else {
        if (!m_transferJob) {
            m_errorText = QLatin1String("save failed");
            setStatus(ErrorWithText);
            return;
        }
        qDebug() << m_transferJob->url();
        emit saved(m_transferJob->url());
    }
    m_transferJob = nullptr;
}
