#ifndef TABLESIZEDIALOG_H
#define TABLESIZEDIALOG_H

#include <QDialog>

namespace Ui {
class TableSizeDialog;
}

class TableSizeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TableSizeDialog(QWidget *parent = nullptr);
    ~TableSizeDialog();
    static QSize getSize(QWidget *parent = nullptr);

private:
    Ui::TableSizeDialog *ui;
};

#endif // TABLESIZEDIALOG_H
