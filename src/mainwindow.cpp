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

    // after all resources for a document are loaded, make the QTextBrowser call its d->relayoutDocument()
    connect(m_document, &Document::allResourcesLoaded,
            [=]() { m_mainWidget->setDocument(m_document); });

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
    qDebug() << url;
    // QUrl::fromUserInput knows how to guess about http and file URLs,
    // but mangles ipfs hashes by converting them to lowercase and setting scheme to http
    bool directory = url.endsWith(QLatin1String("/"));
    if (url.contains(base58HashPrefix) || url.contains(base32HashPrefix))
        m_mainWidget->setSource(QUrl(url), directory ? QTextDocument::MarkdownResource : QTextDocument::UnknownResource);
    else
        m_mainWidget->setSource(QUrl::fromUserInput(url), directory ? QTextDocument::MarkdownResource : QTextDocument::UnknownResource);
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

void MainWindow::on_actionBack_triggered()
{
    m_mainWidget->backward();
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
}
