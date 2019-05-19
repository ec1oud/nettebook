#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QMimeDatabase>
#include <QMimeType>
#include <QTextDocument>
#include <QUrl>

namespace KIO {
class Job;
}

class Document;
class QTextEdit;
class KJob;

class Document : public QTextDocument
{
    Q_OBJECT
public:
    Document(QObject *parent = nullptr);

    void loadUrl(QUrl url);
    QUrl contentUrl();
    QJsonObject filesList(QString url);
    QByteArray jsonDirectoryToMarkdown(QJsonObject j);

signals:
    void status(const QString &text);
    void allResourcesLoaded();

protected:
    QVariant loadResource(int type, const QUrl &name) override;

private slots:
    void resourceDataReceived(KIO::Job *, const QByteArray &data);
    void resourceReceiveDone(KJob *);

private:
    QHash<QUrl, KJob*> m_resourceLoaders;
    QHash<QUrl, QByteArray> m_loadedResources;

    friend class MarkdownBrowser;
};

#endif // DOCUMENT_H
