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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "document.h"

#include <QDebug>
#include <QFileDialog>
#include <QFontDatabase>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeDatabase>
#include <QNetworkReply>
#include <QTextCodec>
#include <QTextDocumentFragment>
#include <QTextEdit>
#include <QTextList>
#include <iostream>
#include <sstream>
#include "jsonview.h"

static const QString ipfsScheme = QStringLiteral("ipfs");
static const QString fileScheme = QStringLiteral("file");
static const QString base58HashPrefix = QStringLiteral("Qm");
static const QString base32HashPrefix = QStringLiteral("bafybei");
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
    connect(m_mainWidget, &QTextEdit::currentCharFormatChanged,
            this, &MainWindow::currentCharFormatChanged);
    connect(m_mainWidget, &QTextEdit::cursorPositionChanged,
            this, &MainWindow::cursorPositionChanged);
    connect(m_mainWidget, &QTextBrowser::anchorClicked,
            this, &MainWindow::updateUrlField);

    // after all resources for a document are loaded, make the QTextBrowser call its d->relayoutDocument()
    connect(m_document, &Document::allResourcesLoaded,
            [=]() { m_mainWidget->setDocument(m_document); });
    connect(m_document, &Document::saved, this, &MainWindow::updateUrlField);

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
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionQuit_triggered()
{
    qApp->quit();
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
    load(fileDialog.selectedFiles().first());
}

void MainWindow::load(QString url)
{
    // QUrl::fromUserInput knows how to guess about http and file URLs,
    // but mangles ipfs hashes by converting them to lowercase and setting scheme to http
    bool directory = url.endsWith(QLatin1String("/"));
    QUrl u(url);
    if (url.contains(base58HashPrefix) || url.contains(base32HashPrefix))
        ; // nothing to do
    else if (u.scheme().isEmpty()) {
        QFileInfo fi(url);
        if (fi.exists())
            u = QUrl::fromLocalFile(fi.canonicalFilePath());
    }
    else
        u = QUrl::fromUserInput(url);
    qDebug() << Q_FUNC_INFO << url << u << "dir?" << directory;
    ui->urlField->setText(u.toString());
    if (url.endsWith("json"))
        showJsonWindow(u); // TODO way less stupid
    else
        m_mainWidget->setSource(u, directory ? QTextDocument::MarkdownResource : QTextDocument::UnknownResource);
}

void MainWindow::on_actionSave_triggered()
{
    qDebug() << m_mainWidget->source();
    m_document->saveAs(m_mainWidget->source());
}

void MainWindow::on_actionSave_As_triggered()
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
        return;
    m_document->saveAs(fileDialog.selectedUrls().first(), fileDialog.selectedMimeTypeFilter());
}

void MainWindow::on_actionSave_to_IPFS_triggered()
{
    m_document->saveToIpfs();
}

bool MainWindow::setBrowserStyle(QUrl url)
{
    // TODO same url resolution as in load()
    if (url.isLocalFile()) {
        qDebug() << "setting style" << url;
        QFile file(url.toLocalFile());
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
}

void MainWindow::updateUrlField(QUrl url)
{
    ui->urlField->setText(url.toString());
}

void MainWindow::showJsonWindow(QUrl url)
{
    JsonView *v = new JsonView();
    qDebug() << Q_FUNC_INFO << url;
    if (url.isLocalFile()) {
        QFile f(url.toLocalFile());
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
        case QTextBlockFormat::NoMarker:
//            actionToggleCheckState->setChecked(false);
            break;
        case QTextBlockFormat::Unchecked:
            ui->styleCB->setCurrentIndex(int(Style::Unchecked));
//            actionToggleCheckState->setChecked(false);
            break;
        case QTextBlockFormat::Checked:
            ui->styleCB->setCurrentIndex(int(Style::Checked));
//            actionToggleCheckState->setChecked(true);
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

void MainWindow::on_styleCB_activated(int index)
{
    if (m_programmaticUiSetting)
        return;
    QTextCursor cursor = m_mainWidget->textCursor();
    QTextListFormat::Style listStyle = QTextListFormat::ListStyleUndefined;
    QTextBlockFormat::MarkerType marker = QTextBlockFormat::NoMarker;
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
        marker = QTextBlockFormat::Unchecked;
        break;
    case Style::Checked:
        if (cursor.currentList())
            listStyle = cursor.currentList()->format().style();
        else
            listStyle = QTextListFormat::ListDisc;
        marker = QTextBlockFormat::Checked;
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

    if (listStyle != QTextListFormat::ListStyleUndefined) {
        QTextListFormat listFmt;
        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }
        listFmt.setStyle(listStyle);
        cursor.createList(listFmt);
    }

    cursor.endEditBlock();
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
    m_hashBegin = text.indexOf(base58HashPrefix);
    if (m_hashBegin >= 0) {
        m_hashEnd = m_hashBegin + 46;
        QUrl apiUrl = m_apiBaseUrl;
        apiUrl.setPath(m_apiBaseUrl.path(QUrl::DecodeReserved) + QLatin1String("cid/base32"));
        apiUrl.setQuery("arg=" + text.mid(m_hashBegin, 46));
        QNetworkReply *reply = m_nam.get(QNetworkRequest(apiUrl));
        connect(reply, &QNetworkReply::finished, [=]() {
            QJsonObject jo = QJsonDocument::fromJson(reply->readAll()).object();
qDebug() << jo;
            reply->deleteLater();
            QString cid = jo.value(QLatin1String("Formatted")).toString();
            QString newText(text);
            ui->urlField->setText(newText.replace(m_hashBegin, 46, cid));
        });
    }
}
