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
    bool loadContent(const QByteArray &content, QMimeType type = QMimeType());

signals:
    void status(const QString &text);
    void allResourcesLoaded();

protected:
    QVariant loadResource(int type, const QUrl &name) override;

private slots:
    void dataReceived(KIO::Job *, const QByteArray &data);
    void dataReceiveDone(KJob *);
    void resourceDataReceived(KIO::Job *, const QByteArray &data);
    void resourceReceiveDone(KJob *);

private:
    QMimeDatabase m_mimeDb;
    QMimeType m_markdownType;
    QByteArray m_rawText;
    QUrl m_contentUrl;
    QUrl m_baseUrl; // redundant, but remembers temporarily from one source to the next
    QHash<QUrl, KJob*> m_resourceLoaders;
    QHash<QUrl, QByteArray> m_loadedResources;

    friend class MarkdownBrowser;
};

#endif // DOCUMENT_H
