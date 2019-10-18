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

    static const QString writingGroup;
    static const QString saveResourcesWithDocuments;
    static const QString resourceDirectorySuffix;
    static const QString journalGroup;
    static const QString journalDirectory;
    static const QString journalFilenameFormat;
    static const QString journalUsesTemplates;
    static const QString styleGroup;
    static const QString codeBlockBackground;

    const QString tasksGroup = QLatin1String("Tasks");
    const QString moveTasksUnderHeading = QLatin1String("moveTasksUnderHeading");
    const QString doneTasksHeadings = QLatin1String("doneTasksHeadings");
    const QString moveTasksToBottom = QLatin1String("moveTasksToBottom");

protected:
    Settings(QObject* parent = nullptr);
    ~Settings();
};

#endif // SETTINGS_H
