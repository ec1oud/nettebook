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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMimeType>
#include <QStack>
#include <QTextBrowser>
#include <QUrl>

namespace Ui {
class MainWindow;
}

class Document;
class QTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void load(QString url);
    bool setBrowserStyle(QUrl url);
//    void loadContent(const QByteArray &content, QMimeType type);

private slots:
    void on_actionQuit_triggered();
    void on_actionOpen_triggered();
    void on_actionBack_triggered();
    void on_browser_backwardAvailable(bool a);
    void on_urlField_returnPressed();
    void on_browser_highlighted(const QUrl &url);

private:
    Ui::MainWindow *ui;
    QTextBrowser *m_mainWidget;
    Document *m_document;
};

#endif // MAINWINDOW_H
