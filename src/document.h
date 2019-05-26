#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QObject>
#include <QJsonDocument>
#include <QMimeDatabase>
#include <QMimeType>
#include <QTextDocument>
#include <QUrl>
#include <KIO/ListJob>

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
    QByteArray fileListMarkdown();

signals:
    void status(const QString &text);
    void allResourcesLoaded();
    void resourceLoaded(QUrl url);

protected:
    QVariant loadResource(int type, const QUrl &name) override;

private slots:
    void resourceDataReceived(KIO::Job *, const QByteArray &data);
    void resourceReceiveDone(KJob *);
    void fileListReceived(KIO::Job *job, const KIO::UDSEntryList &list);

private:
    QHash<QUrl, KJob*> m_resourceLoaders;
    QHash<QUrl, QByteArray> m_loadedResources;
    KIO::UDSEntryList m_fileList;
    QString m_errorText;

    friend class MarkdownBrowser;
};

#endif // DOCUMENT_H
