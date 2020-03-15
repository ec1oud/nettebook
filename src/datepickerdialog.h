#ifndef DATEPICKERDIALOG_H
#define DATEPICKERDIALOG_H

#include <QDate>
#include <QDialog>

namespace Ui {
class DatePickerDialog;
}

class DatePickerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DatePickerDialog(QWidget *parent = nullptr);
    ~DatePickerDialog();
    static QDate choose(QWidget *parent, const QString &title, const QString &message);

private:
    Ui::DatePickerDialog *ui;
};

#endif // DATEPICKERDIALOG_H
