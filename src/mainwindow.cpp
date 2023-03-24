/****************************************************************************
**
** Copyright (C) 2019 Shawn Rutledge
**
** This file is free software; you can redistribute it and/or
** modify it under the terms of the GNU General Public License
** version 3 as published by the Free Software Foundation
** and appearing in the file LICENSE included in the packaging
** of this file.
**
** This code is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
****************************************************************************/

#include "application.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "cidfinder.h"
#include "datepickerdialog.h"
#include "document.h"
#include "ipfsagent.h"
#include "jsonview.h"
#include "kanbancolumnview.h"
#include "settings.h"
#include "settingsdialog.h"
#include "tablesizedialog.h"
#include "tableviewdialog.h"
#include "util.h"

#include <QDebug>
#include <QFileDialog>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>
#include <QMessageBox>
#include <QMimeData>
#include <QMimeDatabase>
#include <QNetworkReply>
#include <QPrintDialog>
#include <QPrinter>
#include <QShortcut>
#include <QStandardPaths>
#include <QTextDocumentFragment>
#include <QTextEdit>
#include <QTextList>
#include <QTextTableCell>
#include <iostream>
#include <sstream>

using namespace Qt::StringLiterals;

Q_DECLARE_LOGGING_CATEGORY(lcWin);

static const auto ipfsScheme = "ipfs"_L1;
static const auto fileScheme = "file"_L1;
static const auto fileModifiedPlaceholder = " [*]"_L1;
static const int BlockQuoteIndent = 40; // pixels, same as in QTextHtmlParserNode::initializeProperties

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_document(new Document(this)),
    m_monoFont(QFontDatabase::systemFont(QFontDatabase::FixedFont))
{
    ui->setupUi(this);
    m_mainWidget = ui->browser;
    m_mainWidget->setDocument(m_document);
    m_highlighter = new CodeBlockHighlighter(m_document);
    connect(m_mainWidget, &QTextEdit::currentCharFormatChanged,
            this, &MainWindow::currentCharFormatChanged);
    connect(m_mainWidget, &QTextEdit::cursorPositionChanged,
            this, &MainWindow::cursorPositionChanged);
    connect(m_mainWidget, &QTextBrowser::sourceChanged,
            this, &MainWindow::updateUrlField);

    // after all resources for a document are loaded, make the QTextBrowser call its d->relayoutDocument()
    connect(m_document, &Document::allResourcesLoaded, m_document,
            [=]() { m_mainWidget->setDocument(m_document); });
    connect(m_document, &Document::saved, this, &MainWindow::updateUrlField);
    connect(m_document, &Document::contentSourceChanged, this, &MainWindow::updateUrlField);
    connect(m_document, &Document::modificationChanged, this, &QWidget::setWindowModified);
    connect(m_document, &Document::undoAvailable, ui->action_Undo, &QAction::setEnabled);
    connect(m_document, &Document::redoAvailable, ui->action_Redo, &QAction::setEnabled);
    connect(m_document, &Document::errorTextChanged, m_document, [this](const QString &text) {
        if (text.isEmpty())
            ui->statusBar->clearMessage();
        else
            ui->statusBar->showMessage(text);
    });
    connect(m_mainWidget, &QTextEdit::copyAvailable, ui->actionCut, &QAction::setEnabled);
    connect(m_mainWidget, &QTextEdit::copyAvailable, ui->action_Copy, &QAction::setEnabled);
    connect(m_mainWidget, &MarkdownBrowser::editLink, this, &MainWindow::on_actionEdit_Link_triggered);
    connect(m_mainWidget, &MarkdownBrowser::unlink, this, &MainWindow::on_actionUnlink_triggered);

    while (ui->toolbarStuff->count()) {
        QWidget *tw = ui->toolbarStuff->takeAt(0)->widget();
        if (!tw)
            continue;
        tw->setAttribute(Qt::WA_AcceptTouchEvents);
        ui->mainToolBar->addWidget(tw);
    }
    delete ui->toolbarStuff;
    ui->toolbarStuff = nullptr;

    while (ui->editToolbarStuff->count()) {
        QWidget *tw = ui->editToolbarStuff->takeAt(0)->widget();
        if (!tw)
            continue;
        tw->setAttribute(Qt::WA_AcceptTouchEvents);
        ui->editToolBar->addWidget(tw);
    }
    delete ui->editToolbarStuff;
    ui->editToolbarStuff = nullptr;

    on_actionToggleEditMode_toggled(false);
    ui->thumbnailsDock->hide();

    ui->searchToolBar->insertWidget(ui->actionFindPrevious, ui->searchField);
    while (ui->searchToolbarStuff->count()) {
        QWidget *tw = ui->searchToolbarStuff->takeAt(0)->widget();
        if (!tw)
            continue;
        tw->setAttribute(Qt::WA_AcceptTouchEvents);
        ui->searchToolBar->addWidget(tw);
    }
    delete ui->searchToolbarStuff;
    ui->searchToolbarStuff = nullptr;
    ui->searchToolBar->hide();

    connect(new QShortcut(QKeySequence(Qt::Key_Escape), this), &QShortcut::activated,
            this, &MainWindow::escape);

#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printdialog)
    ui->actionPrint->setEnabled(true);
#endif
    ui->actionBack->setVisible(!Settings::instance()->boolOrDefault(Settings::readingGroup, Settings::openLinksInNewWindows, true));
    setAcceptDrops(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

bool MainWindow::isEmpty() const
{
    return m_document->isEmpty();
}

bool MainWindow::maybeSave()
{
    if (!m_document->isModified())
        return true;

    switch (QMessageBox::warning(this, QCoreApplication::applicationName(),
            tr("The file has been modified.\n"
            "Do you want to save your changes?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel)) {
    case QMessageBox::Save:
        return on_actionSave_triggered();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::on_actionOpen_triggered()
{
    QFileDialog fileDialog(this, tr("Open a rich text file"));
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setMimeTypeFilters(QStringList()
                                  << "text/html"
                                  << "text/markdown"
                                  << "text/plain");
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    const bool newWindow = m_mainWidget->source().isValid() &&
            Settings::instance()->boolOrDefault(Settings::readingGroup, Settings::openLinksInNewWindows, true);
    if (newWindow) {
        for (const auto &url : fileDialog.selectedUrls())
            static_cast<Application *>(qApp)->load(url);
    } else {
        loadUrl(fileDialog.selectedUrls().constFirst());
    }
}

void MainWindow::on_actionReload_triggered()
{
    if (maybeSave())
        m_mainWidget->reload();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *ev)
{
    if (ev->mimeData()->hasUrls()) {
        load(ev->mimeData()->urls().first().toString());
        ev->acceptProposedAction();
    }
}

void MainWindow::load(QString url)
{
    // QUrl::fromUserInput knows how to guess about http and file URLs,
    // but mangles ipfs hashes by converting them to lowercase and setting scheme to http
    bool directory = url.endsWith("/"_L1);
    QUrl u(url);
    CidFinder::Result cidResult = CidFinder::findIn(url);
    m_jsonDocument = QJsonDocument();
    ui->action_Raw_DAG_node->setEnabled(false);
    bool specialContentLoaded = false;
    if (cidResult.isValid()) {
        // fetch the raw DAG node
        IpfsAgent agent;
        m_jsonDocument = agent.execPost("dag/get", "arg=/ipfs/" + url.mid(cidResult.start) + "&cid-base=base32", {});
        ui->action_Raw_DAG_node->setEnabled(!m_jsonDocument.isEmpty());
        qDebug() << "raw DAG node" << m_jsonDocument.object();
        // deal with special cases like page series
        auto series = m_jsonDocument.object().value("series");
        if (series.isArray()) {
            specialContentLoaded = true;
            newPageSeries();
            int i = 0;
            for (auto v : series.toArray()) {
                QString path = v.toString(); // in practice probably just a CID
                QUrl u("/" + path);
                u.setScheme(ipfsScheme);
#ifndef NETTEBOOK_NO_KIO
                m_ipfsAgent.getFileKIO(u, [path,this,i](QByteArray content) {
                    QString s = QString::fromUtf8(content);
                    if (i == 0)
                        m_document->setMarkdown(s);
                    m_thumbs->append(path, s);
                });
#endif
                ++i;
            }
        }
    } else if (u.scheme().isEmpty()) {
        QFileInfo fi(url);
        if (fi.exists())
            u = QUrl::fromLocalFile(fi.canonicalFilePath());
    } else if (u.scheme() == ipfsScheme) {
        // there is no CID, so we have something like ipfs:local or whatever: probably a directory
        if (u.adjusted(QUrl::RemoveFilename).path().isEmpty()) {
            u.setPath(QLatin1Char('/') + u.path() + QLatin1Char('/'));
            directory = true;
        }
        // TODO resolve and populate m_jsonDocument
    } else {
        u = QUrl::fromUserInput(url);
    }
    qDebug() << Q_FUNC_INFO << url << u << "dir?" << directory;
    ui->urlField->setText(u.toString());
    if (url.endsWith("json"))
        showJsonWindow(u); // TODO way less stupid
    else {
        if (!specialContentLoaded)
            m_mainWidget->setSource(u, directory ? QTextDocument::MarkdownResource : QTextDocument::UnknownResource);
        updateUrlField(u);
    }
}

void MainWindow::loadUrl(const QUrl &url)
{
    m_mainWidget->setSource(url);
    updateUrlField(url);
}

void MainWindow::loadJournal(QStringList dateAndTopics)
{
    // TODO
    // if (Settings::instance()->boolOrDefault(Settings::journalGroup, Settings::journalUsesTemplates, true))
    // loadTemplate("journal"_L1);

    QString dateString = dateAndTopics.isEmpty() ? QString() : dateAndTopics.first();
    QDate date = (dateString.isEmpty() ? QDate::currentDate() : QDate::fromString(dateString, Qt::ISODate));
    if (!date.isValid()) {
        if (dateString == "today"_L1 || dateString == tr("today"))
            date = QDate::currentDate();
        else if (dateString == "yesterday"_L1 || dateString == tr("yesterday"))
            date = QDate::currentDate().addDays(-1);
        else if (dateString == "tomorrow"_L1 || dateString == tr("tomorrow"))
            date = QDate::currentDate().addDays(1);
        else {
            date = QDate::fromString(dateString, "yyyyMMdd"_L1);
            if (!date.isValid())
                date = QDate::fromString(dateString, Qt::RFC2822Date);
            if (!date.isValid()) {
                QLocale locale;
                date = locale.toDate(dateString, QLocale::ShortFormat); // 2-digit years are problematic though: QTBUG-82886
                if (!date.isValid())
                    date = locale.toDate(dateString, QLocale::LongFormat);
            }
        }
    }
    if (!date.isValid()) {
        date = DatePickerDialog::choose(this, tr("Failed to parse date"),
            tr("Sorry but I don't understand\n%1.\nYou can pick a date:").arg(dateString));
    }
    if (!date.isValid()) {
        qWarning() << tr("%1 is not a valid date").arg(dateString);
        return;
    }
    QDir path = Settings::instance()->stringOrDefault(Settings::journalGroup, Settings::journalDirectory,
                                                      QDir::home().filePath("journal"_L1));
    if (!path.exists()) {
        if (QMessageBox::question(this, tr("Create directory?"), tr("No journal directory.  Create?")) == QMessageBox::Yes) {
            if (!QDir::home().mkpath(path.path()))
                ui->statusBar->showMessage(tr("Failed to create %1").arg(path.path()));
        }
    }
    QString filename = Settings::instance()->stringOrDefault(Settings::journalGroup, Settings::journalFilenameFormat, "$date-$topics.md"_L1);
    filename.replace("$date"_L1, date.toString("yyyyMMdd"_L1));
    QString topics;
    if (dateAndTopics.count() > 1) {
        dateAndTopics.removeFirst();
        topics = dateAndTopics.join(filename.contains("-$topics") ? QLatin1Char('-') : QLatin1Char(' '));
    }
    filename.replace("-$topics"_L1, topics.isEmpty() ? topics : QLatin1Char('-') + topics);
    filename.replace("$topics"_L1, topics);
    setEditMode();
    load(path.filePath(filename));
}

void MainWindow::loadTemplate(QString name)
{
    const auto templatesSubdir ="templates"_L1;
    QFileInfo contentFi(m_document->contentSource().fileName());
    QString fileName = name + QLatin1Char('.') + contentFi.suffix();
    QDir documentDir = QFileInfo(Util::toLocalFile(m_document->contentSource())).dir();
    documentDir.cd(".templates"_L1);
qDebug() << "looking for" << fileName << "in" << documentDir << "which started with" << m_document->contentSource() << Util::toLocalFile(m_document->contentSource());
    QFileInfo fi(documentDir, fileName);
    if (!fi.isReadable())
        fi = QFileInfo(QStandardPaths::locate(QStandardPaths::AppConfigLocation,
                                              templatesSubdir + QDir::separator() + fileName));
    if (fi.isReadable()) {
        qDebug() << "loading template" << fi.canonicalFilePath();
        QFile f(fi.canonicalFilePath());
        if (!f.open(QIODevice::ReadOnly))
            return; // should never happen because we pre-checked
        QString content = QString::fromLocal8Bit(f.readAll());
        // actually it's kindof a silly limitation that for a markdown file, only a markdown template can be used, only html for html, etc.
        // but it's more trouble to look for all possible templates with different extensions in different places
        if (contentFi.suffix() == "md"_L1) {
            // workaround for missing QTextCursor::insertMarkdown()
            QTextDocument qtd;
            qtd.setMarkdown(content);
            m_mainWidget->textCursor().insertHtml(qtd.toHtml());
        } else if (contentFi.suffix() == "html"_L1) {
            m_mainWidget->textCursor().insertHtml(content);
        } else {
            m_mainWidget->textCursor().insertText(content);
        }
        m_mainWidget->textCursor().insertBlock(QTextBlockFormat(), QTextCharFormat());
    } else {
        QStringList paths;
        for (const auto &p : QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation))
            paths << p + QDir::separator() + templatesSubdir;
        qWarning() << "failed to find template" << name << "in" << documentDir.path() << "or" << paths;
    }
}

bool MainWindow::isLoaded(QString urlString)
{
    QUrl url = QUrl::fromUserInput(urlString);
    QUrl loadedUrl(ui->urlField->text());
    if (url.scheme() != loadedUrl.scheme())
        return false;
    if (url.isLocalFile() && loadedUrl.isLocalFile()) {
        return QFileInfo(url.toLocalFile()).canonicalFilePath() == QFileInfo(loadedUrl.toLocalFile()).canonicalFilePath();
    }
    return url == loadedUrl;
}

bool MainWindow::on_actionSave_triggered()
{
    qDebug() << m_mainWidget->source();
    if (m_thumbs)
        m_thumbs->saveAllToIpfs();
    else {
        if (m_document->contentSource().isEmpty())
            return on_actionSave_As_triggered();
        else
            m_document->saveAs(m_document->contentSource());
    }
    return true;
}

bool MainWindow::on_actionSave_As_triggered()
{
    qDebug() << m_mainWidget->source();
    QFileDialog fileDialog(this, tr("Save as..."));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setDirectoryUrl(m_document->baseUrl());
    QStringList mimeTypes;
    mimeTypes
#if QT_CONFIG(textmarkdownwriter)
        << "text/markdown"
#endif
        << "text/html"
#if QT_CONFIG(textodfwriter)
        << "application/vnd.oasis.opendocument.text"
#endif
        << "text/plain";
    fileDialog.setMimeTypeFilters(mimeTypes);
#if QT_CONFIG(textmarkdownwriter)
    fileDialog.setDefaultSuffix("md");
#endif
    if (fileDialog.exec() != QDialog::Accepted)
        return false;
    m_document->saveAs(fileDialog.selectedUrls().constFirst(), fileDialog.selectedMimeTypeFilter());
    return true;
}

bool MainWindow::on_actionSave_to_IPFS_triggered()
{
    m_document->saveToIpfs();
    return true;
}

bool MainWindow::setBrowserStyle(QUrl url)
{
    // TODO same url resolution as in load()
    if (url.isLocalFile()) {
        qDebug() << "setting style" << url;
        QFile file(Util::toLocalFile(url));
        if (file.open(QFile::ReadOnly)) {
            QByteArray data = file.readAll();
            m_document->setDefaultStyleSheet(QString::fromLatin1(data));
            return true;
        } else {
            qWarning() << "failed to load stylesheet" << url;
        }
    }
    return false;
}

void MainWindow::setEditMode(bool mode)
{
    ui->actionToggleEditMode->setChecked(mode);
    ui->actionBack->setVisible(!Settings::instance()->boolOrDefault(Settings::readingGroup, Settings::openLinksInNewWindows, true));
}

void MainWindow::updateUrlField(QUrl url)
{
    qCDebug(lcWin) << this << url;
    ui->urlField->setText(url.toString());
    bool filenameOnly = (url.scheme() == ipfsScheme || url.scheme() == fileScheme) && !url.fileName().isEmpty();
    setWindowTitle((filenameOnly ? url.fileName() : url.toString()) + fileModifiedPlaceholder);
    m_document->setModified(false);
    m_mainWidget->updateWatcher();
    if (m_linkDialog)
        m_linkDialog->setDocumentPath(url);
}

void MainWindow::showJsonWindow(QUrl url)
{
    JsonView *v = new JsonView();
    qDebug() << Q_FUNC_INFO << url;
    if (url.isLocalFile()) {
        QFile f(Util::toLocalFile(url));
        if (f.open(QFile::ReadOnly))
            v->load(f.readAll());
        else
            qWarning() << "failed to open" << url;
    }
    // TODO else use KIO (or maybe just use it in the first place)
    v->show();
}

void MainWindow::currentCharFormatChanged(const QTextCharFormat &format)
{
    m_programmaticUiSetting = true;
    ui->actionStrongEmphasis->setChecked(format.font().bold());
    ui->actionEmphasis->setChecked(format.font().italic());
    ui->actionStrikeOut->setChecked(format.font().strikeOut());
    ui->actionMonospace->setChecked(QFontInfo(format.font()).fixedPitch());
//    ui->actionUnderline->setChecked(format.font().underline());
    m_programmaticUiSetting = false;
}

void MainWindow::cursorPositionChanged()
{
    m_programmaticUiSetting = true;
    QTextList *list = m_mainWidget->textCursor().currentList();
    if (list) {
        switch (list->format().style()) {
        case QTextListFormat::ListDisc:
            ui->styleCB->setCurrentIndex(int(Style::BulletDisc));
            break;
        case QTextListFormat::ListCircle:
            ui->styleCB->setCurrentIndex(int(Style::BulletCircle));
            break;
        case QTextListFormat::ListSquare:
            ui->styleCB->setCurrentIndex(int(Style::BulletSquare));
            break;
        case QTextListFormat::ListDecimal:
        case QTextListFormat::ListLowerAlpha:
        case QTextListFormat::ListUpperAlpha:
        case QTextListFormat::ListLowerRoman:
        case QTextListFormat::ListUpperRoman:
            ui->styleCB->setCurrentIndex(int(Style::Numbered));
            break;
        default:
            ui->styleCB->setCurrentIndex(-1);
            break;
        }
        switch (m_mainWidget->textCursor().block().blockFormat().marker()) {
        case QTextBlockFormat::MarkerType::NoMarker:
            ui->actionToggle_Checkbox->setChecked(false);
            break;
        case QTextBlockFormat::MarkerType::Unchecked:
            ui->styleCB->setCurrentIndex(int(Style::Unchecked));
            ui->actionToggle_Checkbox->setChecked(false);
            break;
        case QTextBlockFormat::MarkerType::Checked:
            ui->styleCB->setCurrentIndex(int(Style::Checked));
            ui->actionToggle_Checkbox->setChecked(true);
            break;
        }
    } else {
        int headingLevel = m_mainWidget->textCursor().blockFormat().headingLevel();
        ui->styleCB->setCurrentIndex(headingLevel ? int(Style::Heading) : int(Style::Paragraph));
        if (headingLevel) {
            m_programmaticUiSetting = true;
            ui->headingLevelSB->setValue(headingLevel);
            m_programmaticUiSetting = false;
        }
        if (m_mainWidget->textCursor().blockFormat().hasProperty(QTextFormat::BlockCodeLanguage))
            ui->styleCB->setCurrentIndex(int(Style::CodeBlock));
        int blockQuoteLevel = m_mainWidget->textCursor().blockFormat().intProperty(QTextFormat::BlockQuoteLevel);
        if (blockQuoteLevel)
            ui->styleCB->setCurrentIndex(int(Style::BlockQuote));
        // TODO some of these can be nested, but the combobox is only for mutually-exclusive styles
        ui->actionToggle_Checkbox->setChecked(false);
    }
//    int indentLevel = m_mainWidget->textCursor().blockFormat().indent();
    m_programmaticUiSetting = false;
}

void MainWindow::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = m_mainWidget->textCursor();
    cursor.beginEditBlock();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    m_mainWidget->mergeCurrentCharFormat(format);
    cursor.endEditBlock();
}

void MainWindow::modifyIndentation(int amount)
{
    QTextCursor cursor = m_mainWidget->textCursor();
    cursor.beginEditBlock();
    if (cursor.currentList()) {
        QTextListFormat listFmt = cursor.currentList()->format();
        // See whether the line above is the list we want to move this item into,
        // or whether we need a new list.
        QTextCursor above(cursor);
        above.movePosition(QTextCursor::Up);
        if (above.currentList() && listFmt.indent() + amount == above.currentList()->format().indent()) {
            above.currentList()->add(cursor.block());
        } else {
            listFmt.setIndent(listFmt.indent() + amount);
            cursor.createList(listFmt);
        }
    } else {
        QTextBlockFormat blockFmt = cursor.blockFormat();
        blockFmt.setIndent(blockFmt.indent() + amount);
        cursor.setBlockFormat(blockFmt);
    }
    cursor.endEditBlock();
}

void MainWindow::on_actionBack_triggered()
{
    m_mainWidget->backward();
    updateUrlField(m_mainWidget->source());
}

void MainWindow::on_browser_backwardAvailable(bool a)
{
    ui->actionBack->setEnabled(a);
}

void MainWindow::on_urlField_returnPressed()
{
    load(ui->urlField->text());
}

void MainWindow::on_browser_highlighted(const QUrl &url)
{
    ui->statusBar->showMessage(url.toString());
}

void MainWindow::on_actionToggleEditMode_toggled(bool edit)
{
    qDebug() << edit;
    m_mainWidget->setReadOnly(!edit);
    ui->editToolBar->setVisible(edit);
    ui->actionSave->setVisible(edit);
    ui->actionSave_As->setVisible(edit);
    ui->actionSave_to_IPFS->setVisible(edit);
    ui->menuInsert->setEnabled(edit);
    ui->menuEdit->setEnabled(edit);
}

void MainWindow::on_actionStrongEmphasis_toggled(bool a)
{
    if (m_programmaticUiSetting)
        return;
    QTextCharFormat fmt;
    fmt.setFontWeight(a ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void MainWindow::on_actionEmphasis_toggled(bool a)
{
    if (m_programmaticUiSetting)
        return;
    QTextCharFormat fmt;
    fmt.setFontItalic(a);
    mergeFormatOnWordOrSelection(fmt);
}

void MainWindow::on_actionStrikeOut_toggled(bool a)
{
    if (m_programmaticUiSetting)
        return;
    QTextCharFormat fmt;
    fmt.setFontStrikeOut(a);
    mergeFormatOnWordOrSelection(fmt);
}

void MainWindow::on_actionMonospace_toggled(bool a)
{
    if (m_programmaticUiSetting)
        return;
    QTextCharFormat fmt;
    QFont f = m_mainWidget->textCursor().block().charFormat().font();
    if (a)
        f.setFamily(m_monoFont.family());
    fmt.setFont(f);
    mergeFormatOnWordOrSelection(fmt);
}

void MainWindow::on_actionToggle_Checkbox_triggered(bool checked)
{
    QTextBlock block = m_mainWidget->textCursor().block();
    const bool wasInList = block.textList();
    if (m_mainWidget->textCursor().blockFormat().marker() == QTextBlockFormat::MarkerType::NoMarker)
        on_styleCB_activated(int(Style::Unchecked));
    else
        on_styleCB_activated(int(checked ? Style::Checked : Style::Unchecked));
    if (wasInList)
        m_document->onTaskItemToggled(block, checked);
}

void MainWindow::on_styleCB_activated(int index)
{
    if (m_programmaticUiSetting)
        return;
//qDebug() << index << "BEFORE:";
//m_document->dumpBlocks();
    QTextCursor cursor = m_mainWidget->textCursor();
    QTextListFormat::Style listStyle = QTextListFormat::ListStyleUndefined;
    QTextBlockFormat::MarkerType marker = QTextBlockFormat::MarkerType::NoMarker;
    QTextBlockFormat blockFmt = cursor.blockFormat();
    cursor.beginEditBlock();

    switch (Style(index)) {
    case Style::BulletDisc:
        listStyle = QTextListFormat::ListDisc;
        break;
    case Style::BulletCircle:
        listStyle = QTextListFormat::ListCircle;
        break;
    case Style::BulletSquare:
        listStyle = QTextListFormat::ListSquare;
        break;
    case Style::Unchecked:
        if (cursor.currentList())
            listStyle = cursor.currentList()->format().style();
        else
            listStyle = QTextListFormat::ListDisc;
        marker = QTextBlockFormat::MarkerType::Unchecked;
        break;
    case Style::Checked:
        if (cursor.currentList())
            listStyle = cursor.currentList()->format().style();
        else
            listStyle = QTextListFormat::ListDisc;
        marker = QTextBlockFormat::MarkerType::Checked;
        break;
    case Style::Numbered:
        listStyle = QTextListFormat::ListDecimal;
        break;
    case Style::Heading:
        cursor.endEditBlock();
        on_headingLevelSB_valueChanged(ui->headingLevelSB->value());
        return;
    case Style::Paragraph:
        blockFmt = QTextBlockFormat();
        listStyle = QTextListFormat::ListStyleUndefined;
        if (blockFmt.headingLevel())
            cursor.select(QTextCursor::BlockUnderCursor);
        {
            QTextCharFormat fmt;
            fmt.setFontWeight(QFont::Normal);
            fmt.setProperty(QTextFormat::FontSizeAdjustment, 0);
            cursor.setCharFormat(fmt);
        }
        break;
    case Style::BlockQuote:
        if (!blockFmt.intProperty(QTextFormat::BlockQuoteLevel))
            blockFmt.setProperty(QTextFormat::BlockQuoteLevel, 1);
        blockFmt.setLeftMargin(BlockQuoteIndent);
        blockFmt.setRightMargin(BlockQuoteIndent);
        break;
    case Style::CodeBlock:
        blockFmt.setProperty(QTextFormat::BlockCodeLanguage, QString()); // TODO we need UI to specify it
        blockFmt.setProperty(QTextFormat::BlockCodeFence, '`');
        if (cursor.selection().isEmpty())
            cursor.select(QTextCursor::BlockUnderCursor);
        {
            QTextCharFormat fmt;
            fmt.setFont(m_monoFont);
            cursor.setCharFormat(fmt);
        }
        break;
//    default:
//        break;
    }

//    blockFmt.setObjectIndex(-1); // TODO?
    blockFmt.setMarker(marker);
    cursor.setBlockFormat(blockFmt);

    // If the block shall become a list item but wasn't already in a list, then
    // place it either into an adjacent list if there is one, or create a new list.
    if (listStyle != QTextListFormat::ListStyleUndefined && !cursor.currentList()) {
//        blockFmt.setIndent(0);
        cursor.setBlockFormat(blockFmt);
        QTextCursor explorer = m_mainWidget->textCursor();
        explorer.movePosition(QTextCursor::StartOfBlock);
        explorer.movePosition(QTextCursor::PreviousBlock);
        if (QTextList *adjacentList = explorer.currentList()) {
            adjacentList->add(cursor.block());
        } else {
            explorer = m_mainWidget->textCursor();
            explorer.movePosition(QTextCursor::EndOfBlock);
            explorer.movePosition(QTextCursor::NextBlock);
            if (QTextList *adjacentList = explorer.currentList()) {
                adjacentList->add(cursor.block());
            } else {
                QTextListFormat listFmt;
                listFmt.setStyle(listStyle);
//                listFmt.setIndent(blockFmt.indent() + 1);
                cursor.createList(listFmt);
            }
        }
    }

    cursor.endEditBlock();
    ui->actionToggle_Checkbox->setChecked(marker == QTextBlockFormat::MarkerType::Checked);

//qDebug() << "AFTER";
//m_document->dumpBlocks();
}

void MainWindow::on_headingLevelSB_valueChanged(int headingLevel)
{
    if (m_programmaticUiSetting)
        return;
    QTextCursor cursor = m_mainWidget->textCursor();
    cursor.beginEditBlock();
    QTextBlockFormat blockFmt = cursor.blockFormat();
    blockFmt.setHeadingLevel(headingLevel);
    cursor.setBlockFormat(blockFmt);
    // TODO if using CSS, maybe we don't need this
    int sizeAdjustment = headingLevel ? 4 - headingLevel : 0; // H1 to H6: +3 to -2
    QTextCharFormat fmt;
    fmt.setFontWeight(headingLevel ? QFont::Bold : QFont::Normal);
    fmt.setProperty(QTextFormat::FontSizeAdjustment, sizeAdjustment);
    cursor.select(QTextCursor::BlockUnderCursor);
    cursor.mergeCharFormat(fmt);
    m_mainWidget->mergeCurrentCharFormat(fmt);
    cursor.endEditBlock();
}

void MainWindow::on_actionIndent_triggered()
{
    modifyIndentation(1);
}

void MainWindow::on_actionUnindent_triggered()
{
    modifyIndentation(-1);
}

void MainWindow::on_actionConvert_CID_v0_to_v1_triggered()
{
    QString text = ui->urlField->text();
    CidFinder::Result cidResult = CidFinder::findIn(text);
    // TODO check type
    m_hashBegin = cidResult.start;
    if (m_hashBegin >= 0) {
        m_hashEnd = m_hashBegin + cidResult.length;
        IpfsAgent agent;
        QJsonObject jo = agent.execPost("cid/base32", "arg=" + cidResult.toString(text), {}).object();
        qDebug() << jo;
        QString cid = jo.value("Formatted"_L1).toString();
        QString newText(text);
        ui->urlField->setText(newText.replace(m_hashBegin, cidResult.length, cid));
    }
}

void MainWindow::on_actionInsert_Link_triggered()
{
    if (!m_linkDialog) {
        m_linkDialog = new LinkDialog(this);
        connect(m_linkDialog, &LinkDialog::insert, this, &MainWindow::insertLink);
    }
    m_linkDialog->setSelectedText(m_mainWidget->textCursor().selectedText());
    m_linkDialog->setDocumentPath(m_document->contentSource());
    m_linkDialog->setMode(LinkDialog::Mode::InsertLink);
    m_linkDialog->show();
}

void MainWindow::on_actionEdit_Link_triggered()
{
    if (!m_linkDialog) {
        m_linkDialog = new LinkDialog(this);
        connect(m_linkDialog, &LinkDialog::insert, this, &MainWindow::insertLink);
    }
    m_editingSelection = m_mainWidget->textCursor();
    QTextFragment linkFragment = m_document->fragmentAtCursor(m_editingSelection);
//qDebug() << "link goes from" << linkFragment.position() << "with len" << linkFragment.length() << "cursor is at" << cursor.position();
    m_editingSelection.setPosition(linkFragment.position());
    m_editingSelection.setPosition(linkFragment.position() + linkFragment.length(), QTextCursor::KeepAnchor);
    m_mainWidget->setTextCursor(m_editingSelection);
    m_linkDialog->setDestination(m_editingSelection.charFormat().anchorHref());
    m_linkDialog->setLinkText(m_editingSelection.selectedText());
//qDebug() << "anchor names" << cursor.charFormat().anchorHref() << cursor.charFormat().anchorNames();
    m_linkDialog->setDocumentPath(m_document->contentSource());
    m_linkDialog->setMode(LinkDialog::Mode::EditLink);
    m_linkDialog->show();
}

void MainWindow::on_actionUnlink_triggered()
{
    QTextCursor cursor = m_mainWidget->textCursor();
    QTextFragment linkFragment = m_document->fragmentAtCursor(cursor);
//    qDebug() << "link goes from" << linkFragment.position() << "with len" << linkFragment.length() << "cursor is at" << cursor.position();
    cursor.setPosition(linkFragment.position());
    cursor.setPosition(linkFragment.position() + linkFragment.length(), QTextCursor::KeepAnchor);
    cursor.beginEditBlock();
    QTextCharFormat fmt = cursor.charFormat();
    fmt.clearProperty(QTextFormat::ForegroundBrush);
    fmt.clearProperty(QTextFormat::IsAnchor);
    fmt.clearProperty(QTextFormat::AnchorHref);
    cursor.setCharFormat(fmt);
    cursor.endEditBlock();
}

void MainWindow::insertLink(const QString &destination, const QString &text, const QString &title)
{
    Q_UNUSED(title) // markdown supports it, but QTextDocument doesn't (yet?)
    QTextCursor cursor = (m_linkDialog->mode() == LinkDialog::Mode::EditLink ?
                              m_editingSelection : m_mainWidget->textCursor());
//    qDebug() << destination << text << title;
    cursor.beginEditBlock();
    QTextCharFormat fmt = cursor.charFormat();
    fmt.setForeground(QPalette().link());
    fmt.setAnchor(true);
    fmt.setAnchorHref(destination);
    if (m_linkDialog->mode() == LinkDialog::Mode::InsertImage) {
        QTextImageFormat fmt;
        fmt.setName(destination);
        if (!text.isEmpty())
            fmt.setProperty(QTextFormat::ImageAltText, text);
        if (!title.isEmpty())
            fmt.setProperty(QTextFormat::ImageTitle, title);
        cursor.insertImage(fmt);
    } else if (text == cursor.selectedText()) {
        cursor.setCharFormat(fmt);
    } else {
        cursor.insertText(text, fmt);
    }
    cursor.endEditBlock();
}

void MainWindow::on_actionInsert_Image_triggered()
{
    if (!m_linkDialog) {
        m_linkDialog = new LinkDialog(this);
        connect(m_linkDialog, &LinkDialog::insert, this, &MainWindow::insertLink);
    }
    m_linkDialog->setSelectedText(m_mainWidget->textCursor().selectedText());
    m_linkDialog->setDocumentPath(m_document->contentSource());
    m_linkDialog->setMode(LinkDialog::Mode::InsertImage);
    m_linkDialog->show();
}

void MainWindow::on_actionInsert_Horizontal_Rule_triggered()
{
    QTextCursor cursor = m_mainWidget->textCursor();
    cursor.beginEditBlock();
    cursor.insertBlock();
    cursor.movePosition(QTextCursor::PreviousBlock);
    QTextBlockFormat blockFmt;
    blockFmt.setProperty(QTextFormat::BlockTrailingHorizontalRulerWidth, 1);
    cursor.setBlockFormat(blockFmt);
    cursor.movePosition(QTextCursor::NextBlock);
    cursor.endEditBlock();
}

void MainWindow::on_actionInsert_Table_triggered()
{
    QSize size = TableSizeDialog::getSize(this);
    m_mainWidget->textCursor().insertTable(size.height(), size.width());
}

void MainWindow::on_action_Local_IPFS_files_triggered()
{
    const auto url = "ipfs:local"_L1;
    const bool newWindow = m_mainWidget->source().isValid() &&
            Settings::instance()->boolOrDefault(Settings::readingGroup, Settings::openLinksInNewWindows, true);
    if (newWindow)
        static_cast<Application *>(qApp)->load(url);
    else
        load(url);
}

void MainWindow::on_actionCut_triggered()
{
    if (QWidget *t = qApp->focusWidget())
        t->metaObject()->invokeMethod(t, "cut");
}

void MainWindow::on_action_Copy_triggered()
{
    if (QWidget *t = qApp->focusWidget())
        t->metaObject()->invokeMethod(t, "copy");
}

void MainWindow::on_action_Paste_triggered()
{
    if (QWidget *t = qApp->focusWidget())
        t->metaObject()->invokeMethod(t, "paste");
}

void MainWindow::on_action_Undo_triggered()
{
    if (QWidget *t = qApp->focusWidget())
        t->metaObject()->invokeMethod(t, "undo");
}

void MainWindow::on_action_Redo_triggered()
{
    if (QWidget *t = qApp->focusWidget())
        t->metaObject()->invokeMethod(t, "redo");
}

void MainWindow::on_actionConvert_Table_triggered()
{
    QStringList lines = m_mainWidget->textCursor().selection()
            .toPlainText().split(QLatin1Char('\n'));
    TableViewDialog dlg(this);
    dlg.setTextLines(lines);
    if (dlg.exec())
        convertToTable(lines, dlg.columns(), dlg.rows(), dlg.flow());
}

void MainWindow::convertToTable(QStringList lines, int columns, int rows, QListView::Flow flow)
{
    QTextCursor cursor = m_mainWidget->textCursor();
    cursor.beginEditBlock();
    cursor.removeSelectedText();
    QTextTable *table = cursor.insertTable(rows, columns);
//    qDebug() << "lines" << lines.count() << "rows" << rows << "columns" << columns;
    if (flow == QListView::Flow::TopToBottom) {
        int c = 0;
        while (!lines.isEmpty()) {
            for (int r = 0; r < rows && !lines.isEmpty(); ++r)
                table->cellAt(r, c).firstCursorPosition().insertText(lines.takeFirst());
            c++; // yep
        }
    } else { // LeftToRight
        int r = 0;
        while (!lines.isEmpty()) {
            for (int c = 0; c < columns && !lines.isEmpty(); ++c)
                table->cellAt(r, c).firstCursorPosition().insertText(lines.takeFirst());
            r++;
        }
    }
    cursor.endEditBlock();
}

void MainWindow::on_actionNewDocument_triggered()
{
    static_cast<Application *>(qApp)->newWindow({}, true);
}

void MainWindow::on_actionNewPageSeries_triggered()
{
    newPageSeries();
    m_thumbs->appendBlank();
}

void MainWindow::newPageSeries()
{
    if (m_thumbs)
        m_thumbs->clear();
    else {
        m_thumbs = new ThumbnailScene();
        connect(m_thumbs, &ThumbnailScene::currentPageChanging, m_thumbs,
                [=](ThumbnailItem *it) {
            QString content = m_document->toMarkdown();
            if (!content.isEmpty())
                it->content = content;
        });
        connect(m_thumbs, &ThumbnailScene::currentPageChanged, m_thumbs,
                [=](const QString &source, const QString &content) {
            Q_UNUSED(source)
            m_document->setMarkdown(content);
        });
        connect(m_thumbs, &ThumbnailScene::seriesCidChanged, this, &MainWindow::updateUrlField);
        ui->thumbnailsView->setScene(m_thumbs);
    }
    ui->thumbnailsDock->show();
}

void MainWindow::on_action_Raw_DAG_node_triggered()
{
    JsonView *v = new JsonView();
    v->load(m_jsonDocument.toJson());
    v->show();
}

void MainWindow::on_actionSettings_triggered()
{
    SettingsDialog *d = new SettingsDialog();
    d->show();
}

void MainWindow::on_actionTodays_journal_triggered()
{
    loadJournal();
}

void MainWindow::on_actionPrint_triggered()
{
#if defined(QT_PRINTSUPPORT_LIB) && QT_CONFIG(printdialog)
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (m_mainWidget->textCursor().hasSelection())
        dlg->setOption(QAbstractPrintDialog::PrintSelection);
    if (dlg->exec() == QDialog::Accepted)
        m_mainWidget->print(&printer);
    delete dlg;
#endif
}

void MainWindow::on_actionKanban_triggered()
{
    KanbanColumnView *kv = new KanbanColumnView();
    kv->setDocument(m_document);
    kv->show();
}

void MainWindow::escape()
{
    if (ui->searchField->hasFocus())
        ui->searchToolBar->hide();
}

void MainWindow::doSearch(bool backward)
{
    QTextDocument::FindFlags flags;
    if (backward)
        flags.setFlag(QTextDocument::FindBackward);
    if (ui->searchMatchCaseCB->isChecked())
        flags.setFlag(QTextDocument::FindCaseSensitively);
    if (ui->searchWholeWordsCB->isChecked())
        flags.setFlag(QTextDocument::FindWholeWords);
    ui->browser->find(ui->searchField->text(), flags);
}

void MainWindow::on_actionFind_triggered()
{
    ui->searchToolBar->show();
    ui->searchField->setFocus();
}

void MainWindow::on_searchField_returnPressed()
{
    doSearch();
}

void MainWindow::on_actionFindNext_triggered()
{
    doSearch();
}

void MainWindow::on_actionFindPrevious_triggered()
{
    doSearch(true);
}
