// Copyright (C) 2024 Shawn Rutledge <s@ecloud.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QQmlEngine>
#include <QtQuick/QQuickTextDocument>

class Utils : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit Utils(QObject *parent = nullptr);

    Q_INVOKABLE QUrl rangedAnchorUrl(const QUrl &source, const QString &text);

    Q_INVOKABLE bool insertLink(QQuickTextDocument *doc1, const QUrl &u1,
                                QQuickTextDocument *doc2, const QUrl &u2);
};

#endif // UTILS_H
