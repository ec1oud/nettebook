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
#include "mainwindow.h"
#include "settings.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QUrl>

int main(int argc, char *argv[])
{
    Application app(argc, argv);

    app.setApplicationName("NetteBook");
    app.setOrganizationDomain("nettebook.org");
    QCoreApplication::setApplicationVersion(QLatin1String("0.0.1"));

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
            QCoreApplication::translate("main", "Open the file in kanban mode."));
    parser.addOption(kanbanOption);

    parser.addPositionalArgument(QLatin1String("[url]"), QCoreApplication::translate("main", "Optional filename or URL to open"));

    if (!parser.parse(QCoreApplication::arguments())) {
        qWarning() << parser.errorText();
        exit(1);
    }
    if (parser.isSet(versionOption))
        parser.showVersion();
    if (parser.isSet(helpOption))
        parser.showHelp();

    MainWindow w;
    w.connect(&app, &Application::load, &w, &MainWindow::load);
    if (parser.isSet(editOption))
        w.setEditMode(true);
    bool preloadCss = false;
    if (parser.positionalArguments().count() > 0) {
        QString toLoad = parser.positionalArguments().last();
        if (toLoad.endsWith(QLatin1String(".html")) && parser.isSet(cssOption)) {
            preloadCss = true;
            w.setBrowserStyle(QUrl::fromLocalFile(parser.value(cssOption)));
        }
        if (parser.isSet(journalOption))
            w.loadJournal(parser.positionalArguments());
        else
            w.load(toLoad);
    } else {
        if (parser.isSet(journalOption))
            w.loadJournal();
    }
    if (parser.isSet(templateOption))
        w.loadTemplate(parser.value(templateOption));
    else if (parser.isSet(journalOption) && w.isEmpty() &&
             Settings::instance()->boolOrDefault(Settings::journalGroup, Settings::journalUsesTemplates, true))
        w.loadTemplate(QLatin1String("journal"));
    w.show();
    if (!preloadCss && parser.isSet(cssOption))
        w.setBrowserStyle(QUrl::fromLocalFile(parser.value(cssOption)));

    if (parser.isSet(kanbanOption))
        w.on_actionKanban_triggered();

    return app.exec();
}
