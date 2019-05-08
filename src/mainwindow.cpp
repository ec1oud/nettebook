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
#include <QMimeDatabase>
#include <QTextCodec>
#include <QTextEdit>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mainWidget = ui->browser;

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
    load(fileDialog.selectedUrls().first());
}

bool MainWindow::load(const QUrl &url)
{
    bool success = false;
    if (url.isLocalFile()) {
        QString f = url.toLocalFile();
        QFile file(f);
        if (QFile::exists(f) && file.open(QFile::ReadOnly)) {
            QByteArray data = file.readAll();
            QTextCodec *codec = Qt::codecForHtml(data);
            QString str = codec->toUnicode(data);
            QUrl baseUrl = (f.front() == QLatin1Char(':') ? QUrl(f) : QUrl::fromLocalFile(f)).adjusted(QUrl::RemoveFilename);
            mainWidget->document()->setBaseUrl(baseUrl);
            if (Qt::mightBeRichText(str)) {
                mainWidget->setHtml(str);
                success = true;
            } else {
                QMimeDatabase db;
                success = true;
                if (db.mimeTypeForFileNameAndData(f, data).name() == QLatin1String("text/markdown"))
                    mainWidget->setMarkdown(str);
                else
                    mainWidget->setPlainText(QString::fromLocal8Bit(data));
            }
        }
    } else {
        statusBar()->showMessage(tr("remote loading is not yet implemented"));
        return false;
    }
    if (success)
        statusBar()->showMessage(tr("Opened \"%1\"").arg(url.toString()));
    else
        statusBar()->showMessage(tr("Could not open \"%1\"").arg(url.toString()));
    return success;
}

void MainWindow::on_actionGo_back_triggered()
{
    ui->browser->backward();
}

void MainWindow::on_browser_backwardAvailable(bool a)
{
//    ui->actionGo_back->setEnabled(a);
}
