#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QObject>

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);

signals:
    void load(QString url);

protected:
    bool event(QEvent *ev) override;
};

#endif // APPLICATION_H
