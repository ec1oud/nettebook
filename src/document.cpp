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
#include <QTextFrame>
#include <QTextList>
#include <QTextDocumentFragment>
#include <QTextDocumentWriter>

#ifndef NETTEBOOK_NO_KIO
#include <KIO/Job>
#include <KIO/ListJob>
#endif

#include <iostream>

static const QString ipfsScheme = QStringLiteral("ipfs");
static const QString fileScheme = QStringLiteral("file");

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
        qDebug() << "resolving relative URL" << url << "base" << baseUrl() << "meta" << metaInformation(DocumentUrl);
        url = baseUrl().resolved(url);
    }
#ifndef NETTEBOOK_NO_KIO
    if (m_resourceLoaders.contains(name))
        return QVariant(); // still waiting
    else if (!m_loadedResources.contains(name))
        qDebug() << type << name << url;
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
                    qDebug() << "assuming missing local file" << m_mainFile;
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
        }
#else
        KIO::Job* job = KIO::get(url);
qDebug() << "GET" << name << job;
        connect (job, SIGNAL(data(KIO::Job *, const QByteArray &)),
                 this, SLOT(resourceDataReceived(KIO::Job *, const QByteArray &)));
        connect (job, SIGNAL(result(KJob*)), this, SLOT(resourceReceiveDone(KJob*)));
        m_resourceLoaders.insert(name, job);
#endif
    }
    // Waiting for now, but this function can't block.  We'll be nagged again soon.
    return QVariant();
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
        m_loadedResources[url].append(QByteArray());
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
            setStatus(ErrorWithText);
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
            name += QDir::separator();
        auto hash = f.stringValue(KIO::UDSEntry::UDS_LINK_DEST);
        qint64 size = f.numberValue(KIO::UDSEntry::UDS_SIZE);
        ret += ("|[" + name + "](" + name + ")|" + hash +
                "|" + QLocale().formattedDataSize(size) +
//                "|" + f.stringValue(KIO::UDSEntry::UDS_CREATION_TIME) +
//                "|" + f.stringValue(KIO::UDSEntry::UDS_MIME_TYPE) +
                "|\n").toUtf8();
    }
    qDebug() << ret;
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
            desc << (QLatin1String("<h") + QString::number(bfmt.headingLevel()) + ">");
        if (block.blockFormat().hasProperty(QTextFormat::BlockTrailingHorizontalRulerWidth))
            desc << "horzrule";
        if (block.textList()) {
            if (block.textList() != list) {
                list = block.textList();
                desc << (QLatin1String("<list len=") + QString::number(block.textList()->count()) + ">");
                li = 0;
            } else {
                ++li;
            }
            desc << (QLatin1String("<li i=") + QString::number(li) + '>');
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
        if (!movedToSpecificList && settings->boolOrDefault(settings->tasksGroup, settings->moveTasksToBottom, false)) {
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
        url.setScheme(QLatin1String("file"));
    m_saveUrl = url;
    qDebug() << Q_FUNC_INFO << url << mimeType << dir;
    Settings *settings = Settings::instance();
    if (settings->boolOrDefault(settings->writingGroup, settings->saveResourcesWithDocuments, true)) {
        QString suffix = settings->stringOrDefault(settings->writingGroup, settings->resourceDirectorySuffix, QLatin1String("_resources"));
        saveResources(dir, QFileInfo(url.fileName()).baseName() + suffix);
    }
    QString mt = mimeType;
    if (mimeType.contains(QLatin1String("opendocument")))
        m_saveType = OdtResource;
    else if (mimeType == QLatin1String("text/plain"))
        m_saveType = PlainTextResource;
    else if (mimeType == QLatin1String("text/markdown"))
        m_saveType = MarkdownResource;
    else if (mimeType == QLatin1String("text/html") ||
             mimeType == QLatin1String("application/xhtml+xml"))
        m_saveType = HtmlResource;
    else if (m_saveType == ResourceType::HtmlResource)
        mt = QLatin1String("application/xhtml+xml");
    else if (m_saveType == ResourceType::MarkdownResource)
        mt = QLatin1String("text/markdown");
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
    bool needToCreateDir = !d.exists(subdir);
    QTextCursor cursor(this);
    bool moved = true;
    do {
        QTextCharFormat fmt = cursor.charFormat();
        if (fmt.isImageFormat()) {
            QTextImageFormat ifmt = fmt.toImageFormat();
            if (!QFileInfo(ifmt.name()).exists() && !QUrl(ifmt.name()).isLocalFile()) {
                QVariant image = loadResource(ImageResource, ifmt.name());
                if (image.isValid()) {
                    QUrl url(ifmt.name());
                    QString path = d.absoluteFilePath(url.fileName());
                    qDebug() << "saving image" << ifmt.name() << "->" << path;
                    if (needToCreateDir) {
                        d.mkdir(subdir);
                        d.cd(subdir);
                        needToCreateDir = false;
                    }
                    QFile out(path);
                    if (out.open(QFile::WriteOnly | QIODevice::Truncate)) {
                        out.write(image.toByteArray());
                        out.close();
                        ifmt.setName(subdir + QDir::separator() + url.fileName());
                        cursor.select(QTextCursor::BlockUnderCursor);
                        cursor.setCharFormat(ifmt);
                    }
                }
            }
        }
        moved = cursor.movePosition(QTextCursor::NextBlock);
    } while (moved);
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

#ifndef NETTEBOOK_NO_KIO
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
#endif
