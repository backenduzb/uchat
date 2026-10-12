#pragma once

#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QString>

class Lucide final {
public:
    enum class Icon {
        Menu,
        Search,
        X,
        LogOut,
        Palette,
        ArrowLeft,
        User,
        Check,
        Plus,
        Send,
        MessageCircle,
    };

    static QPixmap pixmap(Icon icon, int size, const QColor& color,
                          qreal strokeWidth = 2.0, qreal devicePixelRatio = 0.0);
    static QIcon icon(Icon icon, const QColor& color, int size = 24,
                      qreal strokeWidth = 2.0);

private:
    static void paintIcon(Icon icon, class QPainter* painter, const class QRectF& rect,
                          const QColor& color, qreal strokeWidth);
};
