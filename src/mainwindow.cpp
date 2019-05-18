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

#include <QDebug>
#include <QFileDialog>
#include <QFontDatabase>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeDatabase>
#include <QTextCodec>
#include <QTextEdit>
#include <iostream>
#include <sstream>
#include <KIO/Job>

static const QString ipfsScheme = QStringLiteral("ipfs");
static const QString fileScheme = QStringLiteral("file");
static const QString base58HashPrefix = QStringLiteral("Qm");
static const QString base32HashPrefix = QStringLiteral("bafybei");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    for (auto m : m_mimeDb.allMimeTypes())
        if (m.name() == QLatin1String("text/markdown")) {
            m_markdownType = m;
            break;
        }
    ui->setupUi(this);
    m_mainWidget = ui->browser;

    while (ui->toolbarStuff->count()) {
        QWidget *tw = ui->toolbarStuff->takeAt(0)->widget();
        if (!tw)
            continue;
        tw->setAttribute(Qt::WA_AcceptTouchEvents);
        ui->mainToolBar->addWidget(tw);
    }
    delete ui->toolbarStuff;
    ui->toolbarStuff = nullptr;
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
    qDebug() << url;
    // QUrl::fromUserInput knows how to guess about http and file URLs,
    // but mangles ipfs hashes by converting them to lowercase and setting scheme to http
    if (url.contains(base58HashPrefix) || url.contains(base32HashPrefix))
        loadUrl(QUrl(url));
    else
        loadUrl(QUrl::fromUserInput(url));
}

void MainWindow::loadUrl(QUrl url)
{
    QString urlString = url.toString();
    qDebug() << url << urlString;
    m_contentUrl = url;
//    int slashIndex = url.indexOf(QLatin1Char('/'), ipfsHashIndex);
//    m_baseUrl = url.left(slashIndex);

    qDebug() << url << m_contentUrl << "baseUrl" << m_baseIsIPFS << m_baseUrl << "relative?" << m_contentUrl.isRelative();
    int base58HashIndex = urlString.indexOf(base58HashPrefix);
    int base32HashIndex = urlString.indexOf(base32HashPrefix);
    if (base58HashIndex >= 0 || base32HashIndex >= 0)
        m_contentUrl.setScheme(ipfsScheme);
    else if (m_contentUrl.scheme().isEmpty())
        m_contentUrl.setScheme(fileScheme);
    if (m_contentUrl.isRelative() && base58HashIndex < 0 && base32HashIndex < 0) {
        if (m_baseIsIPFS) {
            urlString = m_baseUrl.toString() + QLatin1Char('/') + urlString;
            ui->urlField->setText(urlString);
        } else {
            QUrl res = m_contentUrl.resolved(m_mainWidget->document()->baseUrl()); // doesn't work for local files
            qDebug() << url << "base" << m_mainWidget->document()->baseUrl() << "resolved" << res << res.fileName() << m_contentUrl.toString();
            // correct for QUrl::resolved() being broken
//            if (res.fileName() != m_contentUrl.toString())
//                res = QUrl(res.toString() + QLatin1Char('/') + m_contentUrl.toString());
            res.setScheme("file");
            if (res.fileName().isEmpty())
                res.setPath(res.path() + QLatin1Char('/') + url.fileName());
            qDebug() << url << res << res.fileName() << m_contentUrl.toString();
            m_contentUrl = res;
            ui->urlField->setText(m_contentUrl.toString());
        }
    } else {
        ui->urlField->setText(urlString);
    }
    m_history.push(ui->urlField->text());
    m_baseUrl = m_contentUrl.adjusted(QUrl::RemoveFilename);
    m_baseIsIPFS = false;
    m_rawText.clear();
    qDebug() << "URL for KIO:" << m_contentUrl << "baseURL for document:" << m_baseUrl;
    KIO::Job* job = KIO::get(m_contentUrl);
    connect (job, SIGNAL(data(KIO::Job *, const QByteArray &)),
             this, SLOT(dataReceived(KIO::Job *, const QByteArray &)));
    connect (job, SIGNAL(result(KJob*)), this, SLOT(dataReceiveDone(KJob*)));
}

void MainWindow::dataReceived(KIO::Job *,const QByteArray & data )
{
    m_rawText.append(data);
}

void MainWindow::dataReceiveDone(KJob *)
{
    qDebug() << "received" << m_rawText.size();
    loadContent(m_rawText);
}

bool MainWindow::loadContent(const QByteArray &content, QMimeType type)
{
    bool success = true;
    // Stupidly m_mimeDb.mimeTypeForData(content) can't recognize markdown.  With the filename it works.
    if (!type.isValid() || type.name() == QLatin1String("text/plain"))
        type = m_mimeDb.mimeTypeForFileNameAndData(m_contentUrl.fileName(), content);
    qDebug() << m_contentUrl.fileName() << "mime type" << type;
    m_mainWidget->document()->setBaseUrl(m_baseUrl);
    if (type.name() == QLatin1String("text/markdown")) {
        m_mainWidget->setMarkdown(QString::fromUtf8(content));
    } else if (type.name() == QLatin1String("text/html") || type.name() == QLatin1String("application/xhtml+xml")) {
        QTextCodec *codec = Qt::codecForHtml(content);
        QString str = codec->toUnicode(content);
        m_mainWidget->setHtml(str);
    } else if (type.name() == QLatin1String("text/plain")) {
        m_mainWidget->setCurrentFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
        m_mainWidget->setPlainText(QString::fromLocal8Bit(content));
    }
    // TODO load images by writing a "loader" markdown file?
    else
        success = false;
    if (success)
        statusBar()->showMessage(tr("Opened \"%1\"").arg(m_contentUrl.toString()));
    else
        statusBar()->showMessage(tr("Could not open \"%1\"").arg(m_contentUrl.toString()));
    return success;
}

bool MainWindow::setBrowserStyle(QUrl url)
{
    // TODO same url resolution as in load()
    if (url.isLocalFile()) {
        qDebug() << "setting style" << url;
        QFile file(url.toLocalFile());
        if (file.open(QFile::ReadOnly)) {
            QByteArray data = file.readAll();
            m_mainWidget->document()->setDefaultStyleSheet(QString::fromLatin1(data));
            return true;
        } else {
            qWarning() << "failed to load stylesheet" << url;
        }
    }
    return false;
}

void MainWindow::on_actionGo_back_triggered()
{
    // ui->browser->backward(); // doesn't work
    if (m_history.count() > 1) {
        m_history.pop(); // lose the current file
        m_baseUrl.clear();
        m_baseIsIPFS = false;
        load(m_history.pop());
    }
}

void MainWindow::on_browser_backwardAvailable(bool a)
{
    Q_UNUSED(a)
//    ui->actionGo_back->setEnabled(a);
}

void MainWindow::on_urlField_returnPressed()
{
    load(ui->urlField->text());
}

QJsonObject MainWindow::filesList(QString url)
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

QByteArray MainWindow::jsonDirectoryToMarkdown(QJsonObject j)
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

void MainWindow::on_browser_highlighted(const QUrl &url)
{
    ui->statusBar->showMessage(url.toString());
}
