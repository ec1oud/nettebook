#include "linkdialog.h"
#include "ui_linkdialog.h"
#include <QAbstractButton>
#include <QFileDialog>
#include <QPushButton>

LinkDialog::LinkDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LinkDialog)
{
    ui->setupUi(this);
    ui->titleLE->hide();
    ui->titleLabel->hide();
    ui->buttonBox->addButton(tr("Insert"), QDialogButtonBox::ApplyRole);
    if (auto i = qobject_cast<QPushButton *>(ui->buttonBox->buttons().last()))
        i->setDefault(true);
    adjustSize();
    connect(ui->absPathCB, &QCheckBox::toggled, this, &LinkDialog::updateDestinationField);
    connect(ui->buttonBox, &QDialogButtonBox::clicked, this, &LinkDialog::onClicked);
}

LinkDialog::~LinkDialog()
{
    delete ui;
}

void LinkDialog::setDocumentPath(const QUrl &path)
{
    m_source = path;
}

void LinkDialog::setSelectedText(const QString &text)
{
    QUrl destCandidate(text, QUrl::StrictMode);
    if (destCandidate.isValid() &&
            (destCandidate.scheme().startsWith(QLatin1String("http"), Qt::CaseInsensitive) ||
             destCandidate.scheme().startsWith(QLatin1String("ip"), Qt::CaseInsensitive))
            && !destCandidate.fileName().isEmpty()) {
        // If we detect something that looks like an URL, set destination
        m_destination = destCandidate;
        updateDestinationField();
        ui->textLE->clear();
    } else {
        ui->textLE->setText(text);
        ui->destinationLE->clear();
    }
}

void LinkDialog::onClicked(QAbstractButton *button)
{
    if (ui->buttonBox->buttonRole(button) == QDialogButtonBox::ApplyRole)
        emit insert(ui->destinationLE->text(), ui->textLE->text(), ui->titleLE->text());
}

void LinkDialog::on_chooseFileButton_clicked()
{
    m_destination = QFileDialog::getOpenFileUrl(this, tr("Choose the link destination"));
    updateDestinationField();
}

void LinkDialog::updateDestinationField()
{
    if (ui->absPathCB->isChecked() || !m_destination.isLocalFile())
        ui->destinationLE->setText(m_destination.toString());
    else {
        if (!ui->absPathCB->isChecked() && m_source.isLocalFile() && m_destination.isLocalFile()) {
            QDir sdir(QFileInfo(m_source.toLocalFile()).dir());
            m_destination = QUrl::fromLocalFile(sdir.absoluteFilePath(m_destination.toLocalFile()));
            ui->destinationLE->setText(sdir.relativeFilePath(m_destination.toLocalFile()));
//            qDebug() << m_destination << "relative to" << m_source.fileName() << "in" << sdir.path() << "is" << ui->destinationLE->text();
        } else {
            ui->destinationLE->setText(m_destination.fileName());
        }
    }
}

void LinkDialog::on_destinationLE_textEdited(const QString &dest)
{
    m_destination = dest;
}
