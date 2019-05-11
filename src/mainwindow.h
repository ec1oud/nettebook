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
#include <QStack>
#include <QUrl>

namespace Ui {
class MainWindow;
}

class QTextEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    bool load(QString url);
    bool loadUrl(QUrl url);
    bool setBrowserStyle(QUrl url);

private slots:
    void on_actionQuit_triggered();
    void on_actionOpen_triggered();
    void on_actionGo_back_triggered();
    void on_browser_backwardAvailable(bool a);

    void on_urlField_returnPressed();

private:
    Ui::MainWindow *ui;
    QTextEdit *m_mainWidget;
    QStack<QString> m_history; // correct for QTextBrowser history being broken (only for markdown?)
};

#endif // MAINWINDOW_H
