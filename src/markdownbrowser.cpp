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

#include "document.h"
#include "markdownbrowser.h"
#include <QDebug>
#include <QApplication>
#include <QThread>

MarkdownBrowser::MarkdownBrowser(QWidget *parent)
    : QTextBrowser(parent)
{

}

QVariant MarkdownBrowser::loadResource(int type, const QUrl &name)
{
    QVariant ret = static_cast<Document *>(document())->loadResource(type, name);
    qDebug() << type << name << (ret.isNull() ? "fetching" : "got it");
    while (ret.isNull()) {
        ret = static_cast<Document *>(document())->loadResource(type, name);
        qApp->processEvents();
        QThread::usleep(1000); // TODO is that the best we can do to wait for resources?
    }
    return ret;
}
