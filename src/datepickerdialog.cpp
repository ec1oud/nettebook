#include "datepickerdialog.h"
#include "ui_datepickerdialog.h"

DatePickerDialog::DatePickerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DatePickerDialog)
{
    ui->setupUi(this);
}

DatePickerDialog::~DatePickerDialog()
{
    delete ui;
}

QDate DatePickerDialog::choose(QWidget *parent, const QString &title, const QString &message)
{
    DatePickerDialog dlg(parent);
    dlg.setWindowTitle(title);
    dlg.ui->label->setText(message);
    dlg.ui->dateEdit->setDate(QDate::currentDate());
    dlg.adjustSize();
    if (dlg.exec())
        return QDate(dlg.ui->dateEdit->date());
    return QDate();
}
