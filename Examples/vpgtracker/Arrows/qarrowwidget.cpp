#include "qarrowwidget.h"

QArrowWidget::QArrowWidget(QWidget *parent) : QWidget(parent)
{
    qRegisterMetaType<ArrowProps>("ArrowProps");

    // Right arrow polygon vertices
    rightarrowpolygon.push_back(QPointF(0.0,5.0));
    rightarrowpolygon.push_back(QPointF(-1.0,10.0));
    rightarrowpolygon.push_back(QPointF(15.0,0.0));
    rightarrowpolygon.push_back(QPointF(-1.0,-10.0));
    rightarrowpolygon.push_back(QPointF(0.0,-5.0));
    rightarrowpolygon.push_back(QPointF(-25.0,-5.0));
    rightarrowpolygon.push_back(QPointF(-25.0,5.0));
}

void QArrowWidget::updateArrow(const ArrowProps &_arrow)
{
    arrow = _arrow;
    update();
}

void QArrowWidget::paintEvent(QPaintEvent *)
{
    QPainter _painter(this);
    _painter.setRenderHint(QPainter::Antialiasing);
    if(arrow.arrowdirection != ArrowProps::Unknown) {
        __drawArrow(_painter);
    } else {
        QFont _font = font();
        _font.setPointSizeF(_font.pointSizeF()*std::sqrt(rect().height()*rect().height() + rect().width()*rect().width())/250.0);
        _painter.setFont(_font);
        _painter.setPen(QColor(255,255,255,100));
        _painter.drawText(rect(),Qt::AlignCenter,tr("Press spacebar to start session"));
    }
}

void QArrowWidget::__drawArrow(QPainter &_painter)
{
    _painter.fillRect(rect(),arrow.backgroundcolor);
    _painter.setPen(Qt::NoPen);
    _painter.setBrush(arrow.arrowcolor);

    _painter.save();
    _painter.translate(rect().center());
    qreal _scale = std::sqrt(rect().height()*rect().height() + rect().width()*rect().width())/250.0;
    _painter.scale(_scale,_scale);
    switch(arrow.arrowdirection) {
        case ArrowProps::ArrowDirection::Up:
            _painter.rotate(-90.0);
        break;

        case ArrowProps::ArrowDirection::Down:
            _painter.rotate(90.0);
        break;

        case ArrowProps::ArrowDirection::Left:
            _painter.rotate(180.0);
        break;

        default:
            break;
    }
    _painter.drawPolygon(&rightarrowpolygon[0],rightarrowpolygon.size());
    _painter.restore();
}
