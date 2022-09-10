#include "application.h"
#include "mainwindow.h"
#include <QFileOpenEvent>

using namespace Qt::StringLiterals;

Application::Application(int &argc, char **argv) : QApplication(argc, argv)
{
    setApplicationName("NetteBook"_L1);
    setOrganizationDomain("nettebook.org"_L1);
    setApplicationVersion("0.1.0"_L1);
}

void Application::load(const QUrl &url)
{
    MainWindow *w = existingWindow(url.toString());
    if (w) {
        w->activateWindow();
        return;
    }
    w = new MainWindow();
    ++m_loadCount;
    w->loadUrl(url);
    w->show();
}

MainWindow *Application::load(const QString &src, const QString &cssSource, bool editMode)
{
    MainWindow *w = existingWindow(src);
    if (w) {
        w->activateWindow();
    } else {
        w = new MainWindow();
        ++m_loadCount;
        if (!cssSource.isEmpty())
            w->setBrowserStyle(cssSource);
        w->setEditMode(editMode);
        w->load(src);
        w->show();
    }
    return w;
}

void Application::loadJournal(QStringList dateAndTopics)
{
    MainWindow *w = new MainWindow();
    ++m_loadCount;
    w->loadJournal(dateAndTopics);
    w->show();
}

void Application::loadKanban(const QString &src)
{
    MainWindow *w = existingWindow(src);
    if (w) {
        w->activateWindow();
        return;
    }
    w = load(src, QString(), true);
    w->on_actionKanban_triggered();
}

void Application::loadTemplate(QString name)
{
    MainWindow *w = new MainWindow();
    ++m_loadCount;
    w->loadTemplate(name);
    w->show();
}

MainWindow *Application::existingWindow(const QString &urlString)
{
    const QList<QWidget *> topLevelWidgets = QApplication::topLevelWidgets();
    for (QWidget *widget : topLevelWidgets) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin && mainWin->isLoaded(urlString))
            return mainWin;
    }
    return nullptr;
}

void Application::newWindow(const QString &cssSource, bool editMode)
{
    MainWindow *w = new MainWindow();
    if (!cssSource.isEmpty())
        w->setBrowserStyle(cssSource);
    w->setEditMode(editMode);
    w->show();
}

bool Application::event(QEvent *ev)
{
    if (ev->type() == QEvent::FileOpen)
        load(static_cast<QFileOpenEvent *>(ev)->url());
    else
        return QApplication::event(ev);
    return true;
}
