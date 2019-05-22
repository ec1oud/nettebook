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

#include "mainwindow.h"
#include <QApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QUrl>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("NetteBook");
    app.setOrganizationDomain("nettebook.org");
    QCoreApplication::setApplicationVersion(QLatin1String("0.0.1"));

    QCommandLineParser parser;
//    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsPositionalArguments);
    const QCommandLineOption helpOption = parser.addHelpOption();
    const QCommandLineOption versionOption = parser.addVersionOption();

//    QCommandLineOption cssOption({"s", "css"},
    QCommandLineOption cssOption(QStringList() << QStringLiteral("s") << QStringLiteral("css"),
            QCoreApplication::translate("main", "Use the given CSS content style file."), QStringLiteral("path"));
    parser.addOption(cssOption);

    if (!parser.parse(QCoreApplication::arguments())) {
        qWarning() << parser.errorText();
        exit(1);
    }
    if (parser.isSet(versionOption))
        parser.showVersion();
    if (parser.isSet(helpOption))
        parser.showHelp();

    MainWindow w;
    if (parser.positionalArguments().count() > 0)
        w.load(parser.positionalArguments().last());
    w.show();
    if (parser.isSet(cssOption))
        w.setBrowserStyle(QUrl::fromLocalFile(parser.value(cssOption)));

    return app.exec();
}
