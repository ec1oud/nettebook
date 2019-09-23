#ifndef LINKDIALOG_H
#define LINKDIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QUrl>

class QAbstractButton;

namespace Ui {
class LinkDialog;
}

class LinkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LinkDialog(QWidget *parent = nullptr);
    ~LinkDialog();
    void setDocumentPath(const QUrl &path);
    void setSelectedText(const QString &text);
    void setDestination(const QString &text);
    void setLinkText(const QString &text);

    enum class Mode {
        InsertLink,
        EditLink,
        InsertImage
    };
    Q_ENUM(Mode)

    Mode mode() { return m_mode; }
    void setMode(Mode mode);

signals:
    void insert(QString destination, QString text, QString title);

private slots:
    void onClicked(QAbstractButton *button);
    void on_chooseFileButton_clicked();
    void on_destinationLE_textEdited(const QString &dest);

private:
    void updateDestinationField();

private:
    Ui::LinkDialog *ui;
    QPushButton m_insertButton;
    QPushButton *m_okButton = nullptr;
    QUrl m_source;
    QUrl m_destination;
    Mode m_mode = Mode::InsertLink;
};

#endif // LINKDIALOG_H
