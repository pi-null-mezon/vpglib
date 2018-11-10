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
        _painter.drawText(rect(),Qt::AlignCenter,tr("Нажмите на пробел чтобы начать"));
    }
}

void QArrowWidget::__drawArrow(QPainter &_painter)
{
    _painter.fillRect(rect(),arrow.backgroundcolor);
    _painter.setPen(Qt::NoPen);
    _painter.setBrush(arrow.arrowcolor);

    _painter.save();

    switch(arrow.arrowposition) {
        case ArrowProps::PosCenter:
            _painter.translate(rect().center());
            break;
        case ArrowProps::PosTopLeft:
            _painter.translate(rect().center().x()/3.0,rect().center().y()/2.0);
            break;
        case ArrowProps::PosTop:
            _painter.translate(rect().center().x(),rect().center().y()/2.0);
            break;
        case ArrowProps::PosTopRight:
            _painter.translate(rect().center().x()*5.0/3.0,rect().center().y()/2);
            break;
        case ArrowProps::PosRight:
            _painter.translate(rect().center().x()*5.0/3.0,rect().center().y());
            break;
        case ArrowProps::PosBottomRight:
            _painter.translate(rect().center().x()*5.0/3.0,rect().center().y()*3.0/2.0);
            break;
        case ArrowProps::PosBottom:
            _painter.translate(rect().center().x(),rect().center().y()*3.0/2.0);
            break;
        case ArrowProps::PosBottomLeft:
            _painter.translate(rect().center().x()/3.0,rect().center().y()*3.0/2.0);
            break;
        case ArrowProps::PosLeft:
            _painter.translate(rect().center().x()/3.0,rect().center().y());
            break;
    }

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
