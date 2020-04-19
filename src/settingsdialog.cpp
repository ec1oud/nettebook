#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "settings.h"
#include <QFileDialog>

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    Settings *settings = Settings::instance();
    ui->saveResourcesCB->setChecked(settings->boolOrDefault(settings->writingGroup, settings->saveResourcesWithDocuments, true));
    ui->resourcesSuffix->setText(settings->stringOrDefault(settings->writingGroup, settings->resourceDirectorySuffix, QLatin1String("_resources")));
    ui->journalDirectory->setText(settings->stringOrDefault(settings->journalGroup, settings->journalDirectory, QLatin1String("~/journal")));
    ui->journalFilenameFormat->setText(settings->stringOrDefault(settings->journalGroup, settings->journalFilenameFormat, QLatin1String("$date-$topics.md")));
    ui->useJournalTemplates->setChecked(settings->boolOrDefault(settings->journalGroup, settings->journalUsesTemplates, true));
    ui->codeBackgroundColorSwatch->setColor(QColor(settings->stringOrDefault(settings->styleGroup, settings->codeBlockBackground, "#EEE")));
    adjustSize();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_SettingsDialog_accepted()
{
    Settings *settings = Settings::instance();
    settings->setBool(settings->writingGroup, settings->saveResourcesWithDocuments, ui->saveResourcesCB->isChecked());
    settings->setString(settings->writingGroup, settings->resourceDirectorySuffix, ui->resourcesSuffix->text());
    settings->setString(settings->journalGroup, settings->journalDirectory, ui->journalDirectory->text());
    settings->setString(settings->journalGroup, settings->journalFilenameFormat, ui->journalFilenameFormat->text());
    settings->setBool(settings->journalGroup, settings->journalUsesTemplates, ui->useJournalTemplates->checkState());
    settings->setString(settings->styleGroup, settings->codeBlockBackground, ui->codeBackgroundColorSwatch->color().name());
}

void SettingsDialog::on_journalDirectoryChooseButton_clicked()
{
    ui->journalDirectory->setText(QFileDialog::getExistingDirectory(this,
        tr("Choose a directory for the journal"), ui->journalDirectory->text()));
}
