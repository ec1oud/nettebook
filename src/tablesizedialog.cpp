#include "tablesizedialog.h"
#include "ui_tablesizedialog.h"

TableSizeDialog::TableSizeDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TableSizeDialog)
{
    ui->setupUi(this);
}

TableSizeDialog::~TableSizeDialog()
{
    delete ui;
}

QSize TableSizeDialog::getSize(QWidget *parent)
{
    TableSizeDialog dlg(parent);
    if (dlg.exec())
        return QSize(dlg.ui->columnsSB->value(), dlg.ui->rowsSB->value());
    return QSize();
}
