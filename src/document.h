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
    void saveAs(const QUrl &url, const QString &mimeType = QString());
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
    void onSaveDataReq(KIO::Job *job, QByteArray &dest);
    void onSaveDone(KJob *job);

private:
    enum ResourceTypeExt {
        PlainTextResource = UserResource,
        OdtResource
    };

    QHash<QUrl, KJob*> m_resourceLoaders;
    QHash<QUrl, QByteArray> m_loadedResources;
    KIO::UDSEntryList m_fileList;
    QString m_errorText;
    int m_saveType = UnknownResource;
    bool m_saveDone = false;

    friend class MarkdownBrowser;
};

#endif // DOCUMENT_H
