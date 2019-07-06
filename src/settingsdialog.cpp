#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "settings.h"

SettingsDialog::SettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    Settings *settings = Settings::instance();
    ui->saveResourcesCB->setChecked(settings->boolOrDefault(settings->writingGroup, settings->saveResourcesWithDocuments, true));
    ui->resourcesSuffix->setText(settings->stringOrDefault(settings->writingGroup, settings->resourceDirectorySuffix, QLatin1String("_resources")));
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
}
