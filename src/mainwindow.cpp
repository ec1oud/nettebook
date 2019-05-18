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
#include <QMimeDatabase>
#include <QTextCodec>
#include <QTextEdit>
#include <iostream>
#include <sstream>

static const QString ipfsScheme = QStringLiteral("ipfs");
static const QString fileScheme = QStringLiteral("file");
static const QString base58HashPrefix = QStringLiteral("Qm");
static const QString base32HashPrefix = QStringLiteral("bafybei");

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_document(new Document(this))
{
    ui->setupUi(this);
    m_mainWidget = ui->browser;
    m_mainWidget->setDocument(m_document);

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
        m_document->loadUrl(QUrl(url));
    else
        m_document->loadUrl(QUrl::fromUserInput(url));
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

//void MainWindow::loadContent(const QByteArray &content, QMimeType type)
//{
//    bool success = true;
//    if (type.name() == QLatin1String("text/markdown")) {
//        m_mainWidget->setMarkdown(QString::fromUtf8(content));
//    } else if (type.name() == QLatin1String("text/html") || type.name() == QLatin1String("application/xhtml+xml")) {
//        QTextCodec *codec = Qt::codecForHtml(content);
//        QString str = codec->toUnicode(content);
//        m_mainWidget->setHtml(str);
//    } else if (type.name() == QLatin1String("text/plain")) {
//        m_mainWidget->setCurrentFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
//        m_mainWidget->setPlainText(QString::fromLocal8Bit(content));
//    }
//    else
//        success = false;
//    if (success)
//        statusBar()->showMessage(tr("Opened \"%1\"").arg(m_contentUrl.toString()));
//    else
//        statusBar()->showMessage(tr("Could not open \"%1\"").arg(m_contentUrl.toString()));
//}

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

void MainWindow::on_browser_highlighted(const QUrl &url)
{
    ui->statusBar->showMessage(url.toString());
}
