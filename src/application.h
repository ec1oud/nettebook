#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
#include <QObject>
#include <QUrl>

class MainWindow;

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);

public slots:
    void load(const QUrl &url);
    MainWindow *load(const QString &src, const QString &cssSource = {}, bool editMode = false);
    void loadJournal(QStringList dateAndTopics = {});
//    void loadJson(const QUrl &url);
    void loadKanban(const QString &src);
    void loadTemplate(QString name);
    void newWindow(const QString &cssSource, bool editMode);

    int loadCount() const { return m_loadCount; }

protected:
    bool event(QEvent *ev) override;

private:
    int m_loadCount = 0;
};

#endif // APPLICATION_H
