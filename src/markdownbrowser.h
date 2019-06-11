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

#ifndef MARKDOWNBROWSER_H
#define MARKDOWNBROWSER_H

#include <QTextBrowser>
#include <QTimer>

class MarkdownBrowser : public QTextBrowser
{
public:
    MarkdownBrowser(QWidget *parent = nullptr);
    void setSource(const QUrl &name) override;
    QVariant loadResource(int type, const QUrl &name) override;

private:
    QTimer m_loadingTimeout;
};

#endif // MARKDOWNBROWSER_H
