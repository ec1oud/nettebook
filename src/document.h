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
class TransferJob;
}

class Document;
class QTextEdit;
class KJob;

class Document : public QTextDocument
{
    Q_OBJECT
public:
    Document(QObject *parent = nullptr);

    QUrl contentSource() const { return m_mainFile; }
    void saveAs(const QUrl &url, const QString &mimeType = QString());
    void saveToIpfs();
    QByteArray fileListMarkdown();

    enum Status {
        NullStatus = 0,
        LoadingMain = 1,
//        LoadingResources = 2,
        Ready = 3,
        ErrorEmpty = -1,
        ErrorWithText = -2
    };

    Status status() { return m_status; }
    QString errorText() { return m_errorText; }

signals:
    void errorTextChanged(const QString &text);
    void allResourcesLoaded();
    void resourceLoaded(QUrl url);
    void saved(QUrl url);
    void contentSourceChanged(QUrl url);

protected:
    QVariant loadResource(int type, const QUrl &name) override;
    void setStatus(Status s);
    void saveResources(const QUrl &dir, const QString &subdir);

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
    QUrl m_mainFile;
    KIO::UDSEntryList m_fileList;
    QString m_errorText;
    KIO::TransferJob *m_transferJob = nullptr;
    int m_saveType = UnknownResource;
    Status m_status = NullStatus;
    bool m_saveDone = false;

    friend class MarkdownBrowser;
};

#endif // DOCUMENT_H
