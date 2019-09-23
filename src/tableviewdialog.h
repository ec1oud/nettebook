#ifndef TABLEVIEWDIALOG_H
#define TABLEVIEWDIALOG_H

#include <QDialog>
#include <QListView>
#include <QPushButton>

namespace Ui {
class TableViewDialog;
}

class TableViewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TableViewDialog(QWidget *parent = nullptr);
    ~TableViewDialog();

    void setTextLines(const QStringList &l);
    int columns();
    int rows();
    QListView::Flow flow() { return m_flow; }

private:
    void rotate();
    void adjustColumnsForRows();
    void adjustRowsForColumns();
    void updateTableView();

private:
    Ui::TableViewDialog *ui;
    QStringList m_textLines;
    QPushButton m_rotateButton;
    QListView::Flow m_flow = QListView::Flow::TopToBottom;
};

#endif // TABLEVIEWDIALOG_H
