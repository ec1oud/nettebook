#include "tableviewdialog.h"
#include "ui_tableviewdialog.h"
#include <QDebug>

TableViewDialog::TableViewDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TableViewDialog)
{
    ui->setupUi(this);
    m_rotateButton.setText(tr("Rotate")); // TODO set icon
    ui->buttonBox->addButton(&m_rotateButton, QDialogButtonBox::ActionRole);
    connect(&m_rotateButton, &QPushButton::clicked, this, &TableViewDialog::rotate);
    connect(ui->rowsSB, &QSpinBox::editingFinished, this, &TableViewDialog::adjustColumnsForRows);
    connect(ui->colsSB, &QSpinBox::editingFinished, this, &TableViewDialog::adjustRowsForColumns);
}

TableViewDialog::~TableViewDialog()
{
    delete ui;
}

void TableViewDialog::setTextLines(const QStringList &l)
{
    m_textLines = l;
    adjustRowsForColumns();
}

int TableViewDialog::columns()
{
    return ui->colsSB->value();
}

int TableViewDialog::rows()
{
    return ui->rowsSB->value();
}

void TableViewDialog::rotate()
{
    if (m_flow == QListView::Flow::TopToBottom)
        m_flow = QListView::Flow::LeftToRight;
    else
        m_flow = QListView::Flow::TopToBottom;
    int cols = ui->colsSB->value();
    int rows = ui->rowsSB->value();
    ui->rowsSB->setValue(cols);
    ui->colsSB->setValue(rows);
    updateTableView();
}

void TableViewDialog::adjustColumnsForRows()
{
    int rows = ui->rowsSB->value();
    int cols = m_textLines.count() / rows;
    if (m_textLines.count() % rows)
        ++cols;
    ui->colsSB->setValue(cols);
    updateTableView();
}

void TableViewDialog::adjustRowsForColumns()
{
    int cols = ui->colsSB->value();
    int rows = m_textLines.count() / cols;
    if (m_textLines.count() % cols)
        ++rows;
    ui->rowsSB->setValue(rows);
    updateTableView();
}

void TableViewDialog::updateTableView()
{
    int cols = ui->colsSB->value();
    int rows = ui->rowsSB->value();
    QStringList lines = m_textLines;
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(cols);
    ui->tableWidget->setRowCount(rows);
    if (m_flow == QListView::Flow::TopToBottom) {
        int c = 0;
        while (!lines.isEmpty()) {
            for (int r = 0; r < rows && !lines.isEmpty(); ++r)
                ui->tableWidget->setItem(r, c, new QTableWidgetItem(lines.takeFirst()));
            c++; // yep
        }
    } else { // LeftToRight
        int r = 0;
        while (!lines.isEmpty()) {
            for (int c = 0; c < cols && !lines.isEmpty(); ++c)
                ui->tableWidget->setItem(r, c, new QTableWidgetItem(lines.takeFirst()));
            r++;
        }
    }
}
