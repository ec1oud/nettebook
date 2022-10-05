#include "document.h"
#include "cidfinder.h"
#include "settings.h"
#include <QApplication>
#include <QBuffer>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QImage>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTextFrame>
#include <QTextList>
#include <QTextDocumentFragment>
#include <QTextDocumentWriter>
#include <QUrlQuery>

#ifndef NETTEBOOK_NO_KIO
#include <KIO/Job>
#include <KIO/ListJob>
#endif

#include <iostream>

using namespace Qt::StringLiterals;

static const auto httpScheme = "http"_L1;
static const auto ipfsScheme = "ipfs"_L1;
//static const auto fileScheme = "file"_L1;

Q_LOGGING_CATEGORY(lcRes, "org.nettebook.document.resources")

struct BlockUserData : public QTextBlockUserData {
    ~BlockUserData() { }

    QTextList *list;
    QTextDocumentFragment frag;
    int idx;
    int pos;
};

Document::Document(QObject *parent) : QTextDocument(parent)
{
}

QVariant Document::loadResource(int t, const QUrl &name)
{
    qCDebug(lcRes) << t << name;
    QTextDocument::ResourceType type = static_cast<QTextDocument::ResourceType>(t);
    if (m_status >= NullStatus && (type == ResourceType::HtmlResource || type == ResourceType::MarkdownResource))
        setStatus(LoadingMain);
    QUrl url(name);
    QString urlString = url.toString();
    CidFinder::Result cidResult = CidFinder::findIn(urlString);
    if (cidResult.isValid()) {
        url.setScheme(ipfsScheme);
        if (url.adjusted(QUrl::RemoveFilename).path().isEmpty())
            url.setPath(QLatin1Char('/') + url.path());
    } else if (url.isRelative()) {
        qCDebug(lcRes) << "resolving relative URL" << url << "base" << baseUrl() << "meta" << metaInformation(DocumentUrl);
        url = baseUrl().resolved(url);
    }
#ifndef NETTEBOOK_NO_KIO
    if (m_resourceLoaders.contains(name))
        return QVariant(); // still waiting
    else if (!m_loadedResources.contains(name))
        qCDebug(lcRes) << type << name << url;
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
    } else
#endif
    {
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
                    qCDebug(lcRes) << "assuming missing local file" << m_mainFile;
                }
            }
            emit resourceLoaded(name);
            return m_loadedResources.value(name);
        }
        // not cached, so try to load it
#ifdef NETTEBOOK_NO_KIO
        if (name.isLocalFile()) {
            QFile f(name.toLocalFile());
            if (f.open(QFile::ReadOnly)) {
                setStatus(Ready);
                return f.readAll();
            } else {
                auto ret = tr("failed to open file '%1'").arg(name.toLocalFile());
                setStatus(ErrorWithText);
                return ret;
            }
        } else if (name.scheme().startsWith(httpScheme)) {
            if (!m_nam) {
                m_nam = new QNetworkAccessManager(this);
                connect(m_nam, &QNetworkAccessManager::finished, this, &Document::resourceDataReceived);
            }
            QNetworkRequest req(url);
            req.setHeader(QNetworkRequest::UserAgentHeader, qApp->applicationName());
            m_nam->get(req);
            ++m_outstandingRequests;
            // Waiting for now, but this function can't block.  We'll be nagged again soon.
            return QVariant();
        }
#else
        KIO::Job* job = KIO::get(url);
        qCDebug(lcRes) << "GET" << name << job;
        connect (job, SIGNAL(data(KIO::Job *, const QByteArray &)),
                 this, SLOT(resourceDataReceived(KIO::Job *, const QByteArray &)));
        connect (job, SIGNAL(result(KJob*)), this, SLOT(resourceReceiveDone(KJob*)));
        m_resourceLoaders.insert(name, job);
        // Waiting for now, but this function can't block.  We'll be nagged again soon.
        return QVariant();
#endif
    }
    return QTextDocument::loadResource(t, name);
}

void Document::setStatus(Document::Status s)
{
    m_status = s;
    if (s != ErrorWithText)
        m_errorText.clear();
    emit errorTextChanged(m_errorText);
}

#ifndef NETTEBOOK_NO_KIO
void Document::resourceDataReceived(KIO::Job *job, const QByteArray & data)
{
    QUrl url = m_resourceLoaders.key(job);
    // TODO check data for error messages
//qCDebug(lcRes) << job << "received" << data.size() << url;
    if (m_loadedResources.contains(url))
        m_loadedResources[url].append(data);
    else
        m_loadedResources.insert(url, data);
}

void Document::resourceReceiveDone(KJob *job)
{
    QUrl url = m_resourceLoaders.key(job);
    qCDebug(lcRes) << "for" << url.toString() << "got" << m_loadedResources.value(url).size() << "bytes";
    if (!m_loadedResources.value(url).size()) {
        if (m_status == LoadingMain)
            setStatus(ErrorEmpty);
        m_loadedResources[url].append(QByteArray());
    } else {
        QMimeType type = QMimeDatabase().mimeTypeForFileNameAndData(url.fileName(), m_loadedResources[url]);
        qCDebug(lcRes) << "detected mime type" << type.name();
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
        qCDebug(lcRes) << "all resources loaded";
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
            setStatus(ErrorWithText);
        } else {
            setStatus(Ready);
        }
        qCDebug(lcRes) << "got file list" << list.count() << m_fileList;
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
            name += QDir::separator();
        auto hash = f.stringValue(KIO::UDSEntry::UDS_LINK_DEST);
        qint64 size = f.numberValue(KIO::UDSEntry::UDS_SIZE);
        ret += ("|[" + name + "](" + name + ")|" + hash +
                "|" + QLocale().formattedDataSize(size) +
//                "|" + f.stringValue(KIO::UDSEntry::UDS_CREATION_TIME) +
//                "|" + f.stringValue(KIO::UDSEntry::UDS_MIME_TYPE) +
                "|\n").toUtf8();
    }
    qCDebug(lcRes) << ret;
    return ret;
}
#endif

void Document::clearCache(const QUrl &url)
{
    m_loadedResources.remove(url);
}

QTextFragment Document::fragmentAtCursor(const QTextCursor &cursor)
{
    QTextBlock::iterator it;
    for (it = cursor.block().begin(); !(it.atEnd()); ++it) {
        QTextFragment currentFragment = it.fragment();
        if (currentFragment.contains(cursor.position()))
            return currentFragment;
    }
    return QTextFragment();
}

void Document::dumpBlocks() {

    QTextFrame::iterator iterator = rootFrame()->begin();
    QTextFrame *currentFrame = iterator.currentFrame();
    int i = 0;
    QTextList *list = nullptr;
    int li = 0;
    qDebug();
    while (!iterator.atEnd()) {
        if (iterator.currentFrame() != currentFrame)
            qDebug() << "child frame?";
        const QTextBlock block = iterator.currentBlock();
        const auto bfmt = block.blockFormat();
        QStringList desc;
        if (bfmt.headingLevel() > 0)
            desc << ("<h"_L1 + QString::number(bfmt.headingLevel()) + ">");
        if (block.blockFormat().hasProperty(QTextFormat::BlockTrailingHorizontalRulerWidth))
            desc << "horzrule";
        if (block.textList()) {
            if (block.textList() != list) {
                list = block.textList();
                desc << ("<list len="_L1 + QString::number(block.textList()->count()) + ">");
                li = 0;
            } else {
                ++li;
            }
            desc << ("<li i="_L1 + QString::number(li) + '>');
            if (bfmt.marker() == QTextBlockFormat::MarkerType::Unchecked)
                desc << "☐";
            else if (bfmt.marker() == QTextBlockFormat::MarkerType::Checked)
                desc << "🗹";
        }
        desc << block.text().left(20);
        if (block.text().length() > 20)
            desc << QString::fromUtf8("…");
        std::cout << desc.join(' ').toStdString() << std::endl;
        ++iterator;
        ++i;
    }
}

/*!
    React to the user's triggering of the toggle-checkbox action:
    move the task list item in the way settings tell us to.
*/
void Document::onTaskItemToggled(QTextBlock &block, bool checked)
{
    if (checked) {
        Settings *settings = Settings::instance();
        bool movedToSpecificList = false;
        if (settings->boolOrDefault(settings->tasksGroup, settings->moveTasksUnderHeading, false)) {
            // find the list under the appropriate heading
            QStringList destHeadings = settings->stringOrDefault(settings->tasksGroup, settings->doneTasksHeadings)
                    .split(QLatin1Char('\n'), Qt::SkipEmptyParts);
            QTextCursor cursor(this);
            bool moved = true;
            bool foundHeading = false;
            bool listIsNew = false;
            QTextList *destList = nullptr;
            do {
                if (cursor.blockFormat().headingLevel()) {
                    if (foundHeading) {
                        Q_ASSERT(!destList);
                        // next heading after the correct one, with no list found: need to
                        // create a list under the prevous block, whatever it is
                        cursor.movePosition(QTextCursor::PreviousBlock);
                        break;
                    }
                    // so we are still looking for the right heading: is this it?
                    QString headingText = cursor.block().text();
                    // qDebug() << "comparing heading" << headingText << "to" << destHeadings;
                    for (const auto &expected : destHeadings)
                        if (!headingText.compare(expected, Qt::CaseInsensitive)) {
                            foundHeading = true;
                            qDebug() << "found destination heading" << headingText;
                        }
                } else if (foundHeading && !destList && cursor.block().textList()) {
                    qDebug() << "found destination list under heading";
                    destList = cursor.block().textList();
                    break;
                }
                moved = cursor.movePosition(QTextCursor::NextBlock);
            } while (moved);
            if (foundHeading && !destList) {
                qDebug() << "no list under heading; creating one";
                cursor.movePosition(QTextCursor::EndOfBlock);
                destList = cursor.insertList(QTextListFormat::ListDisc);
                listIsNew = true;
            }
            // Now destList should be the one to move this item into, if set.
            // Move the item to the bottom of that list.
            if (destList) {
                moveListItem(block, destList);
                movedToSpecificList = true;
                if (listIsNew) {
                    // delete the empty first item which was created along with the list
                    // (and which stupidly has inherited heading format)
                    cursor.setPosition(destList->item(0).position());
                    destList->removeItem(0);
                    cursor.deleteChar();
                }
            }
        }
        if (!movedToSpecificList && settings->boolOrDefault(settings->tasksGroup, settings->moveTasksToBottom, false)
                && block.textList()->count() > 1) {
            // Just move it to the bottom of the same list
            moveListItem(block, block.textList());
        }
    }
}

/*!
    Move the given \a block to a different list \a toList at index \a position.
    If position is negative it means move it to the end.
    If \a undoable is true, try to remember the block's previous location.
    If the block was not in a list, then this movement cannot be undone.
*/
void Document::moveListItem(QTextBlock &block, QTextList *toList, int position, bool undoable)
{
    if (!toList) {
        qWarning() << "no destination list";
        return;
    }
    auto list = block.textList();
    if (!list) {
        qWarning() << "block wasn't in list:" << block.text();
        return;
    }

    qDebug() << "moving list item" << block.text() << "from list with" << block.textList()->count() << "items to list with" << toList->count() << "items";

    // select and copy
    QTextCursor cursor(block);
    cursor.select(QTextCursor::BlockUnderCursor);
    BlockUserData *bud = new BlockUserData;
    bud->list = list;
    bud->idx = list->itemNumber(block);
    bud->pos = block.position();
    bud->frag = cursor.selection();

    // remove from old location
    list->remove(block);
    cursor.removeSelectedText();

    // insert at new location
    if (position < 0) {
        const int lastItemIdx = toList->count() - 1;
        const int lastItemPos = toList->item(lastItemIdx).position();
        cursor.setPosition(lastItemPos);
        cursor.movePosition(QTextCursor::EndOfBlock);
    } else {
        cursor.setPosition(toList->item(position >= 0 ? position : toList->count() - 1).position());
    }
    cursor.insertFragment(bud->frag);
    toList->add(block);

    // save for undo, if requested
    if (undoable)
        block.setUserData(bud);
    else
        delete bud;
}

void Document::saveAs(QUrl url, const QString &mimeType)
{
    m_saveDone = false;
    QUrl dir = url.adjusted(QUrl::RemoveFilename);
    if (url.isLocalFile())
        url.setPath(QDir(dir.toLocalFile()).absoluteFilePath(url.fileName()));
    else if (url.scheme().isEmpty())
        url.setScheme("file"_L1);
    m_saveUrl = url;
    qDebug() << Q_FUNC_INFO << url << mimeType << dir;
    Settings *settings = Settings::instance();
    if (settings->boolOrDefault(settings->writingGroup, settings->saveResourcesWithDocuments, true)) {
        QString suffix = settings->stringOrDefault(settings->writingGroup, settings->resourceDirectorySuffix, "_resources"_L1);
        saveResources(dir, QFileInfo(url.fileName()).baseName() + suffix);
    }
    QString mt = mimeType;
    if (mimeType.contains("opendocument"_L1))
        m_saveType = OdtResource;
    else if (mimeType == "text/plain"_L1)
        m_saveType = PlainTextResource;
    else if (mimeType == "text/markdown"_L1)
        m_saveType = MarkdownResource;
    else if (mimeType == "text/html"_L1 ||
             mimeType == "application/xhtml+xml"_L1)
        m_saveType = HtmlResource;
    else if (m_saveType == ResourceType::HtmlResource)
        mt = "application/xhtml+xml"_L1;
    else if (m_saveType == ResourceType::MarkdownResource)
        mt = "text/markdown"_L1;
#ifdef NETTEBOOK_NO_KIO
    if (m_saveUrl.isLocalFile()) {
        QFile f(m_saveUrl.toLocalFile());
        if (f.open(QFile::WriteOnly)) {
            QByteArray toWrite;
            prepareWriteBuffer(toWrite);
            if (toWrite.isEmpty())
                qWarning() << "writing empty file";
            if (f.write(toWrite) < 0) {
                m_errorText = tr("failed to write '%1'").arg(m_saveUrl.toLocalFile());
                setStatus(ErrorWithText);
            } else {
                f.close();
                emit saved(m_saveUrl);
                setStatus(Ready);
            }
        } else {
            m_errorText = tr("failed to open for writing '%1'").arg(m_saveUrl.toLocalFile());
            setStatus(ErrorWithText);
        }
    } else {
        m_errorText = tr("saving to non-file URLs is only possible via KIO, so far: '%1'").arg(m_saveUrl.toString());
        setStatus(ErrorWithText);
    }
#else
    KIO::TransferJob *job = KIO::put(url, -1, KIO::Overwrite);
    job->addMetaData("content-type", mt);
    connect (job, SIGNAL(dataReq(KIO::Job *, QByteArray &)),
             this, SLOT(onSaveDataReq(KIO::Job *, QByteArray &)));
    connect (job, SIGNAL(result(KJob*)), this, SLOT(onSaveDone(KJob*)));
#endif
}

void Document::saveResources(const QUrl &dir, const QString &subdir)
{
    QDir d(dir.toLocalFile());
    qCDebug(lcRes) << dir << subdir;
    bool needToCreateDir = !d.exists(subdir);
    QTextCursor cursor(this);
    bool moved = true;
    do {
        QTextCharFormat fmt = cursor.charFormat();
        if (fmt.isImageFormat()) {
            QTextImageFormat ifmt = fmt.toImageFormat();
            qCDebug(lcRes) << "found an image" << ifmt.name();
            if (!QFileInfo::exists(ifmt.name()) && !QUrl(ifmt.name()).isLocalFile()) {
                QVariant image = loadResource(ImageResource, ifmt.name());
                if (image.isValid()) {
                    if (needToCreateDir) {
                        if (!d.mkdir(subdir)) {
                            qWarning() << "failed to create" << subdir << "under" << dir;
                            return;
                        }
                        needToCreateDir = false;
                    }
                    QDir sd(d);
                    if (!sd.cd(subdir))
                        qWarning() << "failed to enter" << subdir << "under" << dir;
                    QUrl url(ifmt.name());
                    QFileInfo fi(sd, url.fileName());
                    if (fi.suffix().isEmpty() && url.hasQuery()) {
                        QUrlQuery uq(url);
                        const auto pairs = uq.queryItems(QUrl::FullyDecoded);
                        qCDebug(lcRes) << fi << ": empty suffix; checking query" << pairs;
                        for (const auto &pair : pairs) {
                            QFileInfo quf(QUrl(pair.second).fileName());
                            if (!quf.suffix().isEmpty() && !quf.baseName().isEmpty()) {
                                fi = QFileInfo(sd, quf.fileName());
                                qCDebug(lcRes) << "replaced with" << fi;
                            }
                        }
                    }
                    QString path = fi.absoluteFilePath();
                    qCDebug(lcRes) << "saving image" << url << url.fileName() << url.query() << "->" << path;
                    QFile out(path);
                    if (out.open(QFile::WriteOnly | QIODevice::Truncate)) {
                        out.write(image.toByteArray());
                        out.close();
                        ifmt.setName(subdir + QDir::separator() + fi.fileName());
                        cursor.select(QTextCursor::BlockUnderCursor);
                        cursor.setCharFormat(ifmt);
                    } else {
                        qWarning() << "failed to write" << path;
                    }
                }
            }
        }
        // NextBlock ought to be good enough, but we miss some inline images that way
        moved = cursor.movePosition(QTextCursor::NextCharacter);
    } while (moved);
    qCDebug(lcRes) << "all known resources:" << m_loadedResources.keys();
}

/*!
    Save to a new file, and emit saved() to provide the hash when done.
*/
void Document::saveToIpfs()
{
#ifndef NETTEBOOK_NO_KIO
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
#endif
}

void Document::prepareWriteBuffer(QByteArray &dest)
{
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
}

#ifdef NETTEBOOK_NO_KIO
void Document::resourceDataReceived(QNetworkReply *reply)
{
//    qDebug() << reply->url() << reply->rawHeaderList();
    m_loadedResources.insert(reply->url(), reply->readAll());
    emit resourceLoaded(reply->url());
    --m_outstandingRequests;
    if (!m_outstandingRequests)
        emit allResourcesLoaded();
}

#else // if not NETTEBOOK_NO_KIO
void Document::onSaveDataReq(KIO::Job *job, QByteArray &dest)
{
    Q_UNUSED(job)
    if (m_saveDone)
        return;
    prepareWriteBuffer(dest);
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
        qDebug() << m_saveUrl;
        emit saved(m_saveUrl);
        setStatus(Ready);
    }
    m_transferJob = nullptr;
    m_saveUrl.clear();
}
#endif // NETTEBOOK_NO_KIO
