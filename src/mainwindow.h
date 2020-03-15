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

#include <QJsonDocument>
#include <QListView>
#include <QMainWindow>
#include <QMimeType>
#include <QNetworkAccessManager>
#include <QStack>
#include <QTextBrowser>
#include <QUrl>
#include "markdownbrowser.h"
#include "ipfsagent.h"
#include "linkdialog.h"
#include "thumbnailscene.h"

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

    void closeEvent(QCloseEvent *event) override;

public slots:
    void load(QString url);
    void loadJournal(QString dateString = QString());
    bool maybeSave();
    bool setBrowserStyle(QUrl url);
    void setEditMode(bool mode = true);
    void updateUrlField(QUrl url);
    void showJsonWindow(QUrl url);

private slots:
    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();
    void on_actionQuit_triggered();
    void on_actionOpen_triggered();
    void on_actionReload_triggered();
    void on_actionBack_triggered();
    void on_browser_backwardAvailable(bool a);
    void on_urlField_returnPressed();
    void on_browser_highlighted(const QUrl &url);
    void on_actionToggleEditMode_toggled(bool edit);
    bool on_actionSave_triggered();
    bool on_actionSave_As_triggered();
    bool on_actionSave_to_IPFS_triggered();

    void on_actionStrongEmphasis_toggled(bool a);
    void on_actionEmphasis_toggled(bool a);
    void on_actionStrikeOut_toggled(bool a);
    void on_actionMonospace_toggled(bool a);
    void on_actionToggle_Checkbox_toggled(bool checked);
    void on_actionInsert_Link_triggered();
    void on_actionEdit_Link_triggered();
    void on_actionUnlink_triggered();
    void on_actionInsert_Image_triggered();
    void on_actionInsert_Horizontal_Rule_triggered();
    void on_actionInsert_Table_triggered();
    void on_actionIndent_triggered();
    void on_actionUnindent_triggered();
    void on_styleCB_activated(int index);
    void on_headingLevelSB_valueChanged(int headingLevel);

    void on_actionConvert_CID_v0_to_v1_triggered();
    void on_action_Local_IPFS_files_triggered();

    void on_actionCut_triggered();
    void on_action_Copy_triggered();
    void on_action_Paste_triggered();
    void on_action_Undo_triggered();
    void on_action_Redo_triggered();
    void on_actionConvert_Table_triggered();

    void on_actionNewPageSeries_triggered();

    void on_action_Raw_DAG_node_triggered();

    void on_actionSettings_triggered();

private:
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void modifyIndentation(int amount);
    void newPageSeries();
    void insertLink(const QString &destination, const QString &text, const QString &title);
    void convertToTable(QStringList lines, int columns, int rows, QListView::Flow flow);

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
    MarkdownBrowser *m_mainWidget;
    Document *m_document;
    QJsonDocument m_jsonDocument;
    ThumbnailScene *m_thumbs = nullptr;
    LinkDialog *m_linkDialog = nullptr;
    QFont m_monoFont;
    IpfsAgent m_ipfsAgent;
    QTextCursor m_editingSelection; // set in special cases only
    int m_hashBegin = 0, m_hashEnd = 0;
    bool m_programmaticUiSetting = false;
};

#endif // MAINWINDOW_H
