#include "IconButton.hpp"

#include <QEnterEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>

#include "animation/AnimationManager.hpp"
#include "theme/ThemeManager.hpp"

namespace {

int clampInt(int value, int lo, int hi) noexcept {
    if (value < lo) {
        return lo;
    }
    if (value > hi) {
        return hi;
    }
    return value;
}

} 

IconButton::IconButton(QWidget* parent, ThemeManager* themeManager)
    : QAbstractButton(parent), m_themeManager(themeManager) {
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::TabFocus);
    setCheckable(false);
    m_currentIconColor = m_iconColor;
    if (m_themeManager) {
        setThemeManager(m_themeManager);
        refreshFromTheme(false);
    }
}

IconButton::IconButton(Lucide::Icon icon, QWidget* parent, ThemeManager* themeManager)
    : IconButton(parent, themeManager) {
    m_icon = icon;
    update();
}

void IconButton::setThemeManager(ThemeManager* themeManager) {
    if (m_themeManager) {
        disconnect(m_themeManager, nullptr, this, nullptr);
    }
    m_themeManager = themeManager;
    if (m_themeManager) {
        connect(m_themeManager, &ThemeManager::themeChanged, this,
                [this]() { refreshFromTheme(false); });
        refreshFromTheme(false);
    }
}

void IconButton::setIcon(Lucide::Icon icon) {
    if (m_icon == icon) {
        return;
    }
    m_icon = icon;
    update();
}

Lucide::Icon IconButton::icon() const noexcept {
    return m_icon;
}

QColor IconButton::iconColor() const noexcept {
    return m_iconColor;
}

void IconButton::setIconColor(const QColor& color) {
    if (!color.isValid()) {
        return;
    }
    m_iconColor = color;
    m_iconHoverColor = color;
    refreshFromTheme(false);
}

QColor IconButton::rippleColor() const noexcept {
    return m_rippleColor;
}

void IconButton::setRippleColor(const QColor& color) {
    if (!color.isValid()) {
        return;
    }
    m_rippleColor = color;
    update();
}

int IconButton::iconSize() const noexcept {
    return m_iconSize;
}

void IconButton::setIconSize(int size) {
    m_iconSize = clampInt(size, 12, 48);
    update();
}

int IconButton::animationDuration() const noexcept {
    return m_animationDuration;
}

void IconButton::setAnimationDuration(int ms) {
    m_animationDuration = clampInt(ms, 80, 400);
}

void IconButton::setShowBadge(bool show) {
    if (m_showBadge == show) {
        return;
    }
    m_showBadge = show;
    update();
}

bool IconButton::showBadge() const noexcept {
    return m_showBadge;
}

void IconButton::setBadgeColor(const QColor& color) {
    if (!color.isValid()) {
        return;
    }
    m_badgeColor = color;
    update();
}

QSize IconButton::sizeHint() const {
    return QSize(40, 40);
}

void IconButton::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (m_rippleActive) {
        const qreal radius = maxRippleRadius(m_rippleOrigin) * m_rippleProgress;
        if (radius > 0.5) {
            QColor ripple = m_rippleColor;
            const int alpha = static_cast<int>(
                static_cast<qreal>(m_rippleColor.alpha()) * (1.0 - m_rippleProgress));
            ripple.setAlpha(clampInt(alpha, 0, 255));
            painter.setPen(Qt::NoPen);
            painter.setBrush(ripple);
            painter.setClipRect(rect());
            painter.drawEllipse(m_rippleOrigin, radius, radius);
            painter.setClipping(false);
        }
    }

    const int side = qMin(width(), height());
    const int iconPx = qMin(m_iconSize, side - 8);
    const QRectF iconRect((width() - iconPx) / 2.0, (height() - iconPx) / 2.0,
                          iconPx, iconPx);
    const QPixmap pm = Lucide::pixmap(m_icon, iconPx, m_currentIconColor, 2.0,
                                      painter.device()->devicePixelRatioF());
    painter.drawPixmap(iconRect.topLeft(), pm);

    if (m_showBadge) {
        const qreal dpr = painter.device()->devicePixelRatioF();
        Q_UNUSED(dpr);
        const qreal dotR = 3.5;
        const QPointF dotCenter = iconRect.topRight() + QPointF(-1.0, 2.0);
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_badgeColor);
        painter.drawEllipse(dotCenter, dotR, dotR);
        painter.setPen(QPen(palette().color(QPalette::Window), 1.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(dotCenter, dotR, dotR);
    }
}

void IconButton::enterEvent(QEnterEvent* event) {
    QAbstractButton::enterEvent(event);
    m_hovered = true;
    startIconAnimation(targetIconColor());
}

void IconButton::leaveEvent(QEvent* event) {
    QAbstractButton::leaveEvent(event);
    m_hovered = false;
    startIconAnimation(targetIconColor());
}

void IconButton::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        startRipple(event->position());
    }
    QAbstractButton::mousePressEvent(event);
    update();
}

void IconButton::mouseReleaseEvent(QMouseEvent* event) {
    QAbstractButton::mouseReleaseEvent(event);
    update();
}

void IconButton::startRipple(const QPointF& origin) {
    AnimationManager::stopAnimation(m_rippleAnimation);
    m_rippleOrigin = origin;
    m_rippleProgress = 0.0;
    m_rippleActive = true;

    m_rippleAnimation = AnimationManager::animateProgress(
        0.0, 1.0, m_rippleDuration, this,
        [this](double progress) {
            m_rippleProgress = progress;
            if (progress >= 1.0) {
                m_rippleActive = false;
            }
            update();
        },
        QEasingCurve(QEasingCurve::OutCubic));
}

void IconButton::startIconAnimation(const QColor& target) {
    if (m_currentIconColor == target && m_iconAnimation.isNull()) {
        update();
        return;
    }
    AnimationManager::stopAnimation(m_iconAnimation);
    const QColor from = m_currentIconColor;
    m_iconAnimation = AnimationManager::animateColorValue(
        from, target, m_animationDuration, this,
        [this](const QColor& color) {
            m_currentIconColor = color;
            update();
        });
}

void IconButton::refreshFromTheme(bool animate) {
    if (!m_themeManager) {
        return;
    }
    const auto palette = m_themeManager->currentPalette();
    m_iconColor = palette.iconColor;
    QColor hover = palette.iconColor;
    if (m_themeManager->currentTheme() == ThemeManager::Theme::Dark) {
        hover = QColor("#FFFFFF");
    } else {
        hover = QColor("#000000");
    }
    m_iconHoverColor = hover;
    m_rippleColor = palette.iconRippleColor;
    m_badgeColor = palette.badgeColor;

    const QColor target = targetIconColor();
    if (animate) {
        startIconAnimation(target);
    } else {
        AnimationManager::stopAnimation(m_iconAnimation);
        m_currentIconColor = target;
        update();
    }
}

QColor IconButton::targetIconColor() const noexcept {
    if (m_hovered || isDown()) {
        return m_iconHoverColor;
    }
    return m_iconColor;
}

qreal IconButton::maxRippleRadius(const QPointF& origin) const noexcept {
    const qreal d1 = qHypot(origin.x(), origin.y());
    const qreal d2 = qHypot(width() - origin.x(), origin.y());
    const qreal d3 = qHypot(origin.x(), height() - origin.y());
    const qreal d4 = qHypot(width() - origin.x(), height() - origin.y());
    return qMax(qMax(d1, d2), qMax(d3, d4));
}
