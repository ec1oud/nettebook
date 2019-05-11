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
#include <QMimeDatabase>
#include <QTextCodec>
#include <QTextEdit>
#include <iostream>
#include <sstream>

static const QString ipfsScheme = QStringLiteral("ipfs");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_ipfsClient("localhost", 5001)
{
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

bool MainWindow::load(QString url)
{
    QUrl urlForm = QUrl(url);
    qDebug() << url << urlForm << "base" << m_baseIsIPFS << m_baseUrl << "relative?" << urlForm.isRelative();
    if (urlForm.isRelative()) {
        if (m_baseIsIPFS) {
            url = m_baseUrl + QLatin1Char('/') + url;
            ui->urlField->setText(url);
        } else {
            QUrl res = urlForm.resolved(m_mainWidget->document()->baseUrl()); // doesn't work for local files
            qDebug() << url << res << res.fileName() << urlForm.toString();
            // correct for QUrl::resolved() being broken
            if (res.fileName() != urlForm.toString())
                res = QUrl(res.toString() + QLatin1Char('/') + urlForm.toString());
            qDebug() << url << res << res.fileName() << urlForm.toString();
            urlForm = res;
            ui->urlField->setText(urlForm.toString());
        }
    } else {
        ui->urlField->setText(url);
    }
    m_baseIsIPFS = false;
    bool success = false;
    int ipfsHashIndex = url.indexOf(QLatin1String("Qm"));
    if (urlForm.isLocalFile()) {
        QString f = urlForm.toLocalFile();
        QFile file(f);
        if (QFile::exists(f) && file.open(QFile::ReadOnly)) {
            QByteArray data = file.readAll();
            QFileInfo fi(file);
            QUrl baseUrl = QUrl::fromLocalFile(fi.absolutePath());
            qDebug() << "base URL" << baseUrl;
            m_mainWidget->document()->setBaseUrl(baseUrl);
            QMimeType type = m_mimeDb.mimeTypeForFileNameAndData(f, data);
            success = loadContent(data, type);
        }
    } else if (ipfsHashIndex >= 0) {
        m_baseIsIPFS = true;
        if (urlForm.scheme().isEmpty())
            urlForm.setScheme(ipfsScheme);
        QUrl base = urlForm.adjusted(QUrl::RemoveFilename);
        int slashIndex = url.indexOf(QLatin1Char('/'), ipfsHashIndex);
        m_baseUrl = url.left(slashIndex);
        qDebug() << "ipfs get" << url << "base" << base << m_baseUrl;
        m_mainWidget->document()->setBaseUrl(base); // doesn't seem to help
        std::stringstream contents;
        m_ipfsClient.FilesGet(url.toLatin1().toStdString(), &contents);
        QByteArray data = QByteArray::fromStdString(contents.str());
        QMimeType type = m_mimeDb.mimeTypeForFileNameAndData(url, data);
        success = loadContent(data, type);
    } else {
        statusBar()->showMessage(tr("scheme is not yet implemented: \"%1\"").arg(url));
        return false;
    }
    if (success) {
        m_history.push(url);
        statusBar()->showMessage(tr("Opened \"%1\"").arg(url));
    } else {
        statusBar()->showMessage(tr("Could not open \"%1\"").arg(url));
    }
    return success;
}

bool MainWindow::loadUrl(QUrl url)
{
    qDebug() << url;
    return load(url.toString());
}

bool MainWindow::loadContent(const QByteArray &content, QMimeType type)
{
    bool success = true;
    QMimeDatabase db;
    if (!type.isValid())
        type = db.mimeTypeForData(content);
    qDebug() << "mime type" << type;
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
