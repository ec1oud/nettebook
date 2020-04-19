#ifndef COLORSWATCH_H
#define COLORSWATCH_H

#include <QWidget>

class ColorSwatch : public QWidget
{
    Q_OBJECT
public:
    explicit ColorSwatch(QWidget *parent = nullptr);
    QColor color() { return m_color; }
    void setColor(QColor color);

signals:

protected:
    void paintEvent(QPaintEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QColor m_color;
};

#endif // COLORSWATCH_H
