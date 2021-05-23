#include "application.h"
#include <QFileOpenEvent>

Application::Application(int &argc, char **argv) : QApplication(argc, argv) {}

bool Application::event(QEvent *ev)
{
    if (ev->type() == QEvent::FileOpen)
        emit load(static_cast<QFileOpenEvent *>(ev)->url().toString());
    else
        return QApplication::event(ev);
    return true;
}
