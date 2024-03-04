// Copyright (C) 2024 Shawn Rutledge <s@ecloud.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "yamldocument.h"

#include <QDebug>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QTextDocument>

#include <yaml-cpp/yaml.h>

Q_LOGGING_CATEGORY(lcYml, "org.nettebook.yaml")

YamlDocument::YamlDocument(QObject *parent)
    : QObject(parent)
{
}

QQuickTextDocument *YamlDocument::document() const
{
    return m_document;
}

void YamlDocument::setDocument(QQuickTextDocument *document)
{
    if (document == m_document)
        return;

    if (m_document)
        disconnect(m_document, &QQuickTextDocument::statusChanged, this, &YamlDocument::parse);
    m_document = document;
    if (m_document)
        connect(m_document, &QQuickTextDocument::statusChanged, this, &YamlDocument::parse);
    emit documentChanged();
}

void YamlDocument::parse()
{
    if (m_document->status() != QQuickTextDocument::Status::Loaded)
        return;
    static QString yaml;
    const QString src = m_document->source().fileName();
    QString yfm = m_document->textDocument()->metaInformation(QTextDocument::FrontMatter);
    bool changeValid = yfm != yaml && !yfm.isEmpty();
    if (changeValid) {
        YAML::Node meta = YAML::Load(yfm.toStdString());
        if (meta["birth"]) {
            QString bds = QString::fromStdString(meta["birth"].as<std::string>());
            m_birth = QDateTime::fromString(bds, Qt::ISODateWithMs);
            qCDebug(lcYml) << src << "birth" << bds << m_birth;
            emit birthChanged();
        }
        if (meta["position"]) {
            auto map = meta["position"].as<std::map<std::string, double>>();
            qCDebug(lcYml) << src << "pos" << map;
            setPosition(QPointF(map["x"], map["y"]));
        }
        yaml = yfm;
    }
    if (m_birth.isNull()) {
        QFileInfo fi(src);
        QDateTime fdt = QDateTime::fromString(fi.baseName(), Qt::ISODateWithMs);
        if (fdt.isValid()) {
            m_birth = fdt;
            qCDebug(lcYml) << src << "birth from filename" << m_birth;
        }
    }
    if (m_position.isNull()) {
        qCDebug(lcYml) << src << "position not set";
        emit needsPosition();
    }
    if (changeValid)
        emit parsed();
}

void YamlDocument::saveToDocument()
{
    YAML::Node meta;
    meta["birth"] = m_birth.toString(Qt::ISODateWithMs).toStdString();
    YAML::Node pos;
    pos["x"] = m_position.x();
    pos["y"] = m_position.y();
    meta["position"] = pos;
    std::stringstream ss;
    ss << meta << std::endl;
    m_document->textDocument()->setMetaInformation(QTextDocument::FrontMatter,
                                                   QString::fromStdString(ss.str()));
}

QDateTime YamlDocument::birth() const
{
    return m_birth;
}

void YamlDocument::setBirth(const QDateTime &newBirth)
{
    if (m_birth == newBirth)
        return;
    m_birth = newBirth;
    emit birthChanged();
}

QPointF YamlDocument::position() const
{
    return m_position;
}

void YamlDocument::setPosition(QPointF newPosition)
{
    if (m_position == newPosition)
        return;
    m_position = newPosition;
    emit positionChanged();
}

#include "moc_yamldocument.cpp"
