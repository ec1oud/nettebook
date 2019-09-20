#ifndef LINKDIALOG_H
#define LINKDIALOG_H

#include <QDialog>
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
    QUrl m_source;
    QUrl m_destination;
};

#endif // LINKDIALOG_H
