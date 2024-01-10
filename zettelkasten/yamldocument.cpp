// Copyright (C) 2024 Shawn Rutledge <s@ecloud.org>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "yamldocument.h"

#include <QDebug>
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
        disconnect(m_document->textDocument(), &QTextDocument::contentsChanged, this, &YamlDocument::parse);
    m_document = document;
    if (m_document)
        connect(m_document->textDocument(), &QTextDocument::contentsChanged, this, &YamlDocument::parse);
    emit documentChanged();
}

QUrl YamlDocument::source() const
{
    return m_source;
}

void YamlDocument::setSource(const QUrl &newSource)
{
    if (m_source == newSource)
        return;
    m_source = newSource;
    emit sourceChanged();
}

void YamlDocument::parse()
{
    static QString yaml;
    QString yfm = m_document->textDocument()->metaInformation(QTextDocument::FrontMatter);
    if (yfm != yaml) {
        YAML::Node meta = YAML::Load(yfm.toStdString());
        if (meta["birth"]) {
            QString bds = QString::fromStdString(meta["birth"].as<std::string>());
            m_birth = QDateTime::fromString(bds, Qt::ISODateWithMs);
            qCDebug(lcYml) << m_source.fileName() << "birth" << bds << m_birth;
            emit birthChanged();
        }
        yaml = yfm;
    }
}

void YamlDocument::saveToDocument()
{
    YAML::Node meta;
    meta["birth"] = m_birth.toString(Qt::ISODateWithMs).toStdString();
    std::stringstream ss;
    ss << "---" << std::endl;
    ss << meta << std::endl;
    ss << "---" << std::endl;
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

#include "moc_yamldocument.cpp"
