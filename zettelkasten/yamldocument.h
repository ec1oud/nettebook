// Copyright (C) 2024 Shawn Rutledge <s@ecloud.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef YAMLDOCUMENT_H
#define YAMLDOCUMENT_H

#include <QObject>
#include <QtQuick/QQuickTextDocument>
#include <QQmlEngine>

class YamlDocument : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QQuickTextDocument *document READ document WRITE setDocument NOTIFY documentChanged)
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged FINAL)
    Q_PROPERTY(QDateTime birth READ birth WRITE setBirth NOTIFY birthChanged FINAL)
    Q_PROPERTY(QPointF position READ position WRITE setPosition NOTIFY positionChanged FINAL)
    QML_ELEMENT

public:
    explicit YamlDocument(QObject *parent = nullptr);

    QQuickTextDocument *document() const;
    void setDocument(QQuickTextDocument *document);

    Q_INVOKABLE void saveToDocument();

    QUrl source() const;
    void setSource(const QUrl &newSource);

    QDateTime birth() const;
    void setBirth(const QDateTime &newBirth);

    QPointF position() const;
    void setPosition(QPointF newPosition);

signals:
    void sourceChanged();
    void documentChanged();
    void birthChanged();
    void positionChanged();

private:
    void parse();

private:
    QQuickTextDocument *m_document = nullptr;
    QDateTime m_birth;
    QUrl m_source;
    QPointF m_position;
};

#endif // YAMLDOCUMENT_H
