#include "colorswatch.h"

#include <QColorDialog>
#include <QPainter>

ColorSwatch::ColorSwatch(QWidget *parent) :
    QWidget(parent),
    m_color(QPalette().window().color())
{
    setFixedSize(24, 24);
}

void ColorSwatch::setColor(QColor color)
{
    if (!color.isValid())
        return;
    m_color = color;
    update();
}

void ColorSwatch::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.fillRect(rect(), m_color);
    painter.setPen(QPalette().buttonText().color());
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}

void ColorSwatch::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    setColor(QColorDialog::getColor(m_color, parentWidget(), tr("Code block background")));
}
