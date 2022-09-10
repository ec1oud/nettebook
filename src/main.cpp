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

#include "application.h"
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QUrl>

using namespace Qt::StringLiterals;

int main(int argc, char *argv[])
{
    Application app(argc, argv);

    QCommandLineParser parser;
    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

    QCommandLineOption cssOption(QStringList() << QStringLiteral("s") << QStringLiteral("css"),
            QCoreApplication::translate("main", "Use the given CSS content style file."), QStringLiteral("path"));
    parser.addOption(cssOption);

    QCommandLineOption templateOption(QStringList() << QStringLiteral("t") << QStringLiteral("template"),
                                      QCoreApplication::translate("main", "Use the given template file."), QStringLiteral("name"));
    parser.addOption(templateOption);

    QCommandLineOption editOption(QStringList() << QStringLiteral("e") << QStringLiteral("edit"),
            QCoreApplication::translate("main", "Open the file in edit mode."));
    parser.addOption(editOption);

    QCommandLineOption journalOption(QStringList() << QStringLiteral("j") << QStringLiteral("journal"),
            QCoreApplication::translate("main",
                "Edit a journal entry: today's by default, or give a date (yyyymmdd or yyyy-mm-dd) "
                "or 'today', 'yesterday' or 'tomorrow'.  Topics or keywords can be added after the date."));
    parser.addOption(journalOption);

    QCommandLineOption kanbanOption(QStringLiteral("kanban"),
            QCoreApplication::translate("main", "Open the file(s) in kanban mode."));
    parser.addOption(kanbanOption);

    parser.addPositionalArgument("[url]"_L1, QCoreApplication::translate("main", "Optional filename or URL to open"));

    if (!parser.parse(QCoreApplication::arguments())) {
        qWarning() << parser.errorText();
        exit(1);
    }
    if (parser.isSet(versionOption))
        parser.showVersion();
    if (parser.isSet(helpOption))
        parser.showHelp();

    const QStringList posArgs = parser.positionalArguments();
    for (const QString &url : posArgs) {
        if (parser.isSet(kanbanOption))
            app.loadKanban(url);
        else
            app.load(url, parser.value(cssOption), parser.isSet(editOption));
    }

    if (parser.isSet(journalOption))
        app.loadJournal();
    if (parser.isSet(templateOption))
        app.loadTemplate(parser.value(templateOption));

    if (!app.loadCount())
        app.newWindow(parser.value(cssOption), parser.isSet(editOption));

    return app.exec();
}
