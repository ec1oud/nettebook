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
    MainWindow *w = new MainWindow();
    ++m_loadCount;
    w->loadUrl(url);
    w->show();
}

MainWindow *Application::load(const QString &src, const QString &cssSource, bool editMode)
{
    MainWindow *w = new MainWindow();
    ++m_loadCount;
    if (!cssSource.isEmpty())
        w->setBrowserStyle(cssSource);
    w->setEditMode(editMode);
    w->load(src);
    w->show();
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
    MainWindow *w = load(src, QString(), true);
    w->on_actionKanban_triggered();
}

void Application::loadTemplate(QString name)
{
    MainWindow *w = new MainWindow();
    ++m_loadCount;
    w->loadTemplate(name);
    w->show();
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
