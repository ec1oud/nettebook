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
#include <QDebug>

const QString Settings::writingGroup(QLatin1String("Writing"));
const QString Settings::saveResourcesWithDocuments(QLatin1String("saveResourcesWithDocuments"));
const QString Settings::resourceDirectorySuffix(QLatin1String("resourceDirectorySuffix"));
const QString Settings::journalGroup(QLatin1String("Journal"));
const QString Settings::journalDirectory(QLatin1String("journalDirectory"));
const QString Settings::journalFilenameFormat(QLatin1String("journalFilenameFormat"));

Settings* Settings::instance()
{
    static Settings* self = new Settings();
    return self;
}

Settings::Settings(QObject* parent) :
    QSettings(parent)
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
