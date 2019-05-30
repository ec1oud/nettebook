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
    void setEditMode(bool mode = true);

private slots:
    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();
    void on_actionQuit_triggered();
    void on_actionOpen_triggered();
    void on_actionBack_triggered();
    void on_browser_backwardAvailable(bool a);
    void on_urlField_returnPressed();
    void on_browser_highlighted(const QUrl &url);
    void on_actionToggleEditMode_toggled(bool edit);
    void on_actionSave_triggered();
    void on_actionSave_As_triggered();

    void on_actionStrongEmphasis_toggled(bool a);
    void on_actionEmphasis_toggled(bool a);
    void on_actionStrikeOut_toggled(bool a);
    void on_actionMonospace_toggled(bool a);
    void on_actionIndent_triggered();
    void on_actionUnindent_triggered();
    void on_styleCB_activated(int index);
    void on_headingLevelSB_valueChanged(int headingLevel);

private:
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void modifyIndentation(int amount);

    // to be kept in sync with items in ui->styleCB
    enum class Style {
        Heading = 0,
        Paragraph,
        BlockQuote,
        CodeBlock,
        BulletDisc,
        BulletCircle,
        BulletSquare,
        Checked,
        Unchecked,
        Numbered
    };

private:
    Ui::MainWindow *ui;
    QTextBrowser *m_mainWidget;
    Document *m_document;
    QFont m_monoFont;
    bool m_programmaticUiSetting = false;
};

#endif // MAINWINDOW_H
