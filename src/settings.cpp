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

#include "settings.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

using namespace Qt::StringLiterals;

const QString Settings::readingGroup("Reading"_L1);
const QString Settings::writingGroup("Writing"_L1);
const QString Settings::openLinksInNewWindows("openLinksInNewWindows"_L1);
const QString Settings::saveResourcesWithDocuments("saveResourcesWithDocuments"_L1);
const QString Settings::resourceDirectorySuffix("resourceDirectorySuffix"_L1);
const QString Settings::journalGroup("Journal"_L1);
const QString Settings::journalDirectory("journalDirectory"_L1);
const QString Settings::journalFilenameFormat("journalFilenameFormat"_L1);
const QString Settings::journalUsesTemplates("journalUsesTemplates"_L1);
const QString Settings::styleGroup("Style"_L1);
const QString Settings::codeBlockBackground("codeBlockBackground"_L1);
const QString Settings::searchResultBackground("searchResultBackground"_L1);
const QString Settings::behaviorGroup("Behavior"_L1);
const QString Settings::searchHighlightAll("searchHighlightAll"_L1);

Settings* Settings::instance()
{
    static Settings* self = new Settings();
    return self;
}

Settings::Settings(QObject* parent) :
#ifdef Q_OS_LINUX
    // correct for QTBUG-82888
    QSettings(QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation) + QDir::separator() +
              QCoreApplication::applicationName() + ".conf"_L1, QSettings::NativeFormat, parent)
#else
    QSettings(parent)
#endif
{
    qDebug() << "settings stored at" << fileName();
}

Settings::~Settings()
{
}

bool Settings::boolOrDefault(QString group, QString key, bool defaultVal)
{
    beginGroup(group);
    QVariant v = value(key);
    bool ret = defaultVal;
    if (v.canConvert<bool>())
        ret = v.toBool();
    endGroup();
    return ret;
}

void Settings::setBool(QString group, QString key, bool val)
{
    beginGroup(group);
    setValue(key, QVariant(val));
    endGroup();
}

int Settings::intOrDefault(QString group, QString key, int defaultVal)
{
    beginGroup(group);
    QVariant v = value(key);
    bool ok = false;
    int ret = v.toInt(&ok);
    if (!ok)
        ret = defaultVal;
    endGroup();
    return ret;
}

void Settings::setInt(QString group, QString key, int val)
{
    beginGroup(group);
    setValue(key, QVariant(val));
    endGroup();
}

QString Settings::stringOrDefault(QString group, QString key, QString defaultVal)
{
    beginGroup(group);
    QVariant v = value(key, QVariant(defaultVal));
    endGroup();
    return v.toString();
}

void Settings::setString(QString group, QString key, QString val)
{
    beginGroup(group);
    setValue(key, QVariant(val));
    endGroup();
}
