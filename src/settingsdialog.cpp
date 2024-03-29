#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "settings.h"
#include <QFileDialog>

using namespace Qt::StringLiterals;

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    Settings *settings = Settings::instance();
    ui->openLinksInNewWindowsCB->setChecked(settings->boolOrDefault(settings->readingGroup, settings->openLinksInNewWindows, true));
    ui->saveResourcesCB->setChecked(settings->boolOrDefault(settings->writingGroup, settings->saveResourcesWithDocuments, true));
    ui->resourcesSuffix->setText(settings->stringOrDefault(settings->writingGroup, settings->resourceDirectorySuffix, "_resources"_L1));
    ui->journalDirectory->setText(settings->stringOrDefault(settings->journalGroup, settings->journalDirectory, "~/journal"_L1));
    ui->journalFilenameFormat->setText(settings->stringOrDefault(settings->journalGroup, settings->journalFilenameFormat, "$date-$topics.md"_L1));
    ui->useJournalTemplates->setChecked(settings->boolOrDefault(settings->journalGroup, settings->journalUsesTemplates, true));
    ui->codeBackgroundColorSwatch->setColor(QColor(settings->stringOrDefault(settings->styleGroup, settings->codeBlockBackground, "#EEE"_L1)));
    ui->searchResultBackgroundColorSwatch->setColor(QColor(settings->stringOrDefault(settings->styleGroup, settings->searchResultBackground, "#CFF"_L1)));
    ui->moveTasksUnderHeadingCB->setChecked(settings->boolOrDefault(settings->tasksGroup, settings->moveTasksUnderHeading, true));
    ui->doneTasksHeadingsTE->setPlainText(settings->stringOrDefault(settings->tasksGroup, settings->doneTasksHeadings, tr("Done\nFerdig\nFinito")));
    ui->moveTasksToBottomCB->setChecked(settings->boolOrDefault(settings->tasksGroup, settings->moveTasksToBottom, true));
    adjustSize();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

void SettingsDialog::on_SettingsDialog_accepted()
{
    Settings *settings = Settings::instance();
    settings->setBool(settings->readingGroup, settings->openLinksInNewWindows, ui->openLinksInNewWindowsCB->isChecked());
    settings->setBool(settings->writingGroup, settings->saveResourcesWithDocuments, ui->saveResourcesCB->isChecked());
    settings->setString(settings->writingGroup, settings->resourceDirectorySuffix, ui->resourcesSuffix->text());
    settings->setString(settings->journalGroup, settings->journalDirectory, ui->journalDirectory->text());
    settings->setString(settings->journalGroup, settings->journalFilenameFormat, ui->journalFilenameFormat->text());
    settings->setBool(settings->journalGroup, settings->journalUsesTemplates, ui->useJournalTemplates->checkState());
    settings->setString(settings->styleGroup, settings->codeBlockBackground, ui->codeBackgroundColorSwatch->color().name());
    settings->setString(settings->styleGroup, settings->searchResultBackground, ui->searchResultBackgroundColorSwatch->color().name());
    settings->setBool(settings->tasksGroup, settings->moveTasksUnderHeading, ui->moveTasksUnderHeadingCB->isChecked());
    settings->setString(settings->tasksGroup, settings->doneTasksHeadings, ui->doneTasksHeadingsTE->toPlainText());
    settings->setBool(settings->tasksGroup, settings->moveTasksToBottom, ui->moveTasksToBottomCB->isChecked());
}

void SettingsDialog::on_journalDirectoryChooseButton_clicked()
{
    ui->journalDirectory->setText(QFileDialog::getExistingDirectory(this,
        tr("Choose a directory for the journal"), ui->journalDirectory->text()));
}
