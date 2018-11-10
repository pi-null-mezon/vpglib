#ifndef QARROWWIDGET_H
#define QARROWWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QColor>

struct ArrowProps {
    enum ArrowDirection {Unknown,Left,Up,Right,Down};
    enum ArrowPosition {PosTopLeft, PosTop, PosTopRight, PosRight, PosBottomRight, PosBottom, PosBottomLeft, PosLeft, PosCenter};

    ArrowProps(const QColor &_arrowcolor=Qt::white, const QColor &_backgroundcolor=Qt::gray, ArrowDirection _arrowdirection=Unknown, ArrowPosition _arrowposition=PosCenter):
        arrowcolor(_arrowcolor),
        backgroundcolor(_backgroundcolor),
        arrowdirection(_arrowdirection),
        arrowposition(_arrowposition) {}

    ArrowProps(const ArrowProps &val) :
        arrowcolor(val.arrowcolor),
        backgroundcolor(val.backgroundcolor),
        arrowdirection(val.arrowdirection),
        arrowposition(val.arrowposition) {}

    ArrowProps& operator=(const ArrowProps &val)
    {
        arrowcolor = val.arrowcolor;
        backgroundcolor = val.backgroundcolor;
        arrowdirection = val.arrowdirection;
        arrowposition = val.arrowposition;
        return *this;
    }

    // Move semantic
    ArrowProps(ArrowProps &&val) :
        arrowcolor(qMove(val.arrowcolor)),
        backgroundcolor(qMove(val.backgroundcolor)),
        arrowdirection(val.arrowdirection),
        arrowposition(val.arrowposition) {}

    ArrowProps& operator=(ArrowProps &&val)
    {
        arrowcolor = qMove(val.arrowcolor);
        backgroundcolor = qMove(val.backgroundcolor);
        arrowdirection = val.arrowdirection;
        arrowposition = val.arrowposition;
        return *this;
    }

    static ArrowDirection str2direction(const QString &_str)
    {
        if(_str.contains("Up"))
            return Up;
        if(_str.contains("Down"))
            return Down;
        if(_str.contains("Left"))
            return Left;
        if(_str.contains("Right"))
            return Right;
        return Unknown;
    }

    static QString direction2str(ArrowDirection _direction)
    {
        switch(_direction) {
            case Up:
                return "Up";
            case Down:
                return "Down";
            case Left:
                return "Left";
            case Right:
                return "Right";
            default:
                return "Unknown";
        }
    }

    static ArrowPosition str2position(const QString &_str)
    {
        if(_str == "7")
            return PosTopLeft;
        if(_str == "8")
            return  PosTop;
        if(_str == "9")
            return PosTopRight;
        if(_str == "6")
            return PosRight;
        if(_str == "3")
            return PosBottomRight;
        if(_str == "2")
            return PosBottom;
        if(_str == "1")
            return PosBottomLeft;
        if(_str == "4")
            return PosLeft;
        return PosCenter;
    }

    QColor arrowcolor;
    QColor backgroundcolor;
    ArrowDirection arrowdirection;
    ArrowPosition arrowposition;
};

class QArrowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QArrowWidget(QWidget *parent = nullptr);

signals:

public slots:
    void updateArrow(const ArrowProps &_arrow);

protected:
    void paintEvent(QPaintEvent *);

private:
    void __drawArrow(QPainter &_painter);

    QVector<QPointF> rightarrowpolygon;
    ArrowProps arrow;
};

#endif // QARROWWIDGET_H
