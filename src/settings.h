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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QRectF>
#include <QStringList>

class Settings : public QSettings
{
    Q_OBJECT

public:
    static Settings* instance();

    bool boolOrDefault(QString group, QString key, bool defaultVal);
    void setBool(QString group, QString key, bool val);
    int intOrDefault(QString group, QString key, int defaultVal);
    void setInt(QString group, QString key, int val);
    QString stringOrDefault(QString group, QString key, QString defaultVal = QString());
    void setString(QString group, QString key, QString val);

    const QString writingGroup = QLatin1String("Writing");
    const QString saveResourcesWithDocuments = QLatin1String("saveResourcesWithDocuments");
    const QString resourceDirectorySuffix = QLatin1String("resourceDirectorySuffix");

protected:
    Settings(QObject* parent = nullptr);
    ~Settings();
};

#endif // SETTINGS_H