#include "Lucide.hpp"

#include <QPainter>
#include <QPainterPath>
#include <QRectF>

QPixmap Lucide::pixmap(Icon icon, int size, const QColor& color,
                       qreal strokeWidth, qreal devicePixelRatio) {
    const int px = size <= 0 ? 24 : size;
    qreal dpr = devicePixelRatio;
    if (dpr <= 0.0) {
        dpr = 1.0;
    }
    QPixmap pm(static_cast<int>(static_cast<qreal>(px) * dpr),
               static_cast<int>(static_cast<qreal>(px) * dpr));
    pm.setDevicePixelRatio(dpr);
    pm.fill(Qt::transparent);

    QPainter painter(&pm);
    painter.setRenderHint(QPainter::Antialiasing, true);
    paintIcon(icon, &painter, QRectF(0, 0, px, px), color, strokeWidth);
    painter.end();
    return pm;
}

QIcon Lucide::icon(Icon icon, const QColor& color, int size, qreal strokeWidth) {
    QIcon result;
    result.addPixmap(pixmap(icon, size, color, strokeWidth));
    return result;
}

void Lucide::paintIcon(Icon icon, QPainter* painter, const QRectF& rect,
                       const QColor& color, qreal strokeWidth) {
    if (!painter) {
        return;
    }
    QPen pen(color.isValid() ? color : QColor("#FFFFFF"), strokeWidth,
             Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);

    painter->save();
    painter->translate(rect.topLeft());
    const qreal sx = rect.width() / 24.0;
    const qreal sy = rect.height() / 24.0;
    painter->scale(sx, sy);

    switch (icon) {
    case Icon::Menu: {
        painter->drawLine(QPointF(4, 6), QPointF(20, 6));
        painter->drawLine(QPointF(4, 12), QPointF(20, 12));
        painter->drawLine(QPointF(4, 18), QPointF(20, 18));
        break;
    }
    case Icon::Search: {
        painter->drawEllipse(QPointF(11, 11), 8, 8);
        painter->drawLine(QPointF(16.65, 16.65), QPointF(21, 21));
        break;
    }
    case Icon::X: {
        painter->drawLine(QPointF(18, 6), QPointF(6, 18));
        painter->drawLine(QPointF(6, 6), QPointF(18, 18));
        break;
    }
    case Icon::LogOut: {
        QPainterPath bracket;
        bracket.moveTo(9, 21);
        bracket.lineTo(5, 21);
        bracket.cubicTo(3.9, 21, 3, 20.1, 3, 19);
        bracket.lineTo(3, 5);
        bracket.cubicTo(3, 3.9, 3.9, 3, 5, 3);
        bracket.lineTo(9, 3);
        painter->drawPath(bracket);
        painter->drawLine(QPointF(16, 17), QPointF(21, 12));
        painter->drawLine(QPointF(21, 12), QPointF(16, 7));
        painter->drawLine(QPointF(21, 12), QPointF(9, 12));
        break;
    }
    case Icon::Palette: {
        painter->drawEllipse(QPointF(12, 12), 9, 9);
        painter->setBrush(pen.color());
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(8.5, 10), 1.1, 1.1);
        painter->drawEllipse(QPointF(12, 7.5), 1.1, 1.1);
        painter->drawEllipse(QPointF(15.5, 10), 1.1, 1.1);
        painter->drawEllipse(QPointF(10.5, 14.5), 1.1, 1.1);
        break;
    }
    case Icon::ArrowLeft: {
        painter->drawLine(QPointF(19, 12), QPointF(5, 12));
        painter->drawLine(QPointF(12, 19), QPointF(5, 12));
        painter->drawLine(QPointF(5, 12), QPointF(12, 5));
        break;
    }
    case Icon::User: {
        painter->drawEllipse(QPointF(12, 8), 4, 4);
        QPainterPath shoulders;
        shoulders.moveTo(5, 21);
        shoulders.lineTo(5, 19);
        shoulders.cubicTo(5, 16.8, 6.8, 15, 9, 15);
        shoulders.lineTo(15, 15);
        shoulders.cubicTo(17.2, 15, 19, 16.8, 19, 19);
        shoulders.lineTo(19, 21);
        painter->drawPath(shoulders);
        break;
    }
    case Icon::Check: {
        painter->drawLine(QPointF(20, 6), QPointF(9, 17));
        painter->drawLine(QPointF(9, 17), QPointF(4, 12));
        break;
    }
    case Icon::Plus: {
        painter->drawLine(QPointF(12, 5), QPointF(12, 19));
        painter->drawLine(QPointF(5, 12), QPointF(19, 12));
        break;
    }
    case Icon::Send: {
        QPainterPath plane;
        plane.moveTo(22, 2);
        plane.lineTo(15, 22);
        plane.lineTo(11, 13);
        plane.lineTo(2, 9);
        plane.closeSubpath();
        painter->drawPath(plane);
        painter->drawLine(QPointF(22, 2), QPointF(11, 13));
        break;
    }
    case Icon::MessageCircle: {
        QPainterPath bubble;
        bubble.moveTo(7.9, 20);
        bubble.cubicTo(6.0, 20, 4.6, 18.9, 4.2, 17.2);
        bubble.lineTo(3.0, 12.0);
        bubble.cubicTo(3.0, 7.0, 7.0, 3.5, 12.0, 3.5);
        bubble.cubicTo(17.0, 3.5, 21.0, 7.0, 21.0, 12.0);
        bubble.cubicTo(21.0, 17.0, 17.0, 20.5, 12.0, 20.5);
        bubble.lineTo(7.9, 20);
        bubble.closeSubpath();
        painter->drawPath(bubble);
        break;
    }
    }
    painter->restore();
}
