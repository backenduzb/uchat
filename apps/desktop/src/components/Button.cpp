#include "Button.hpp"

#include <QEnterEvent>
#include <QFocusEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <qtimer.h>

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

} // namespace

Button::Button(QWidget* parent, ThemeManager* themeManager)
    : QPushButton(parent), m_themeManager(themeManager) {
    setCursor(Qt::PointingHandCursor);
    setFlat(false);

    m_loadingTimer = new QTimer(this);
    m_loadingTimer->setInterval(16);
    connect(m_loadingTimer, &QTimer::timeout, this, [this]() {
        m_loadingAngle += 10.0;
        if (m_loadingAngle >= 360.0) {
            m_loadingAngle -= 360.0;
        }
        update();
    });

    if (m_themeManager) {
        const auto palette = m_themeManager->currentPalette();
        m_normalBackground = palette.buttonBackground;
        m_hoverBackground = palette.buttonHover;
        m_pressedBackground = palette.buttonPressed;
        m_disabledBackground = palette.buttonDisabled;
        m_rippleColor = palette.rippleColor;
        m_currentBackground = targetBackground();
        setThemeManager(m_themeManager);
    }
}

Button::Button(const QString& text, QWidget* parent, ThemeManager* themeManager)
    : Button(parent, themeManager) {
    setText(text);
}

void Button::setThemeManager(ThemeManager* themeManager) {
    if (m_themeManager) {
        disconnect(m_themeManager, nullptr, this, nullptr);
    }
    m_themeManager = themeManager;
    if (m_themeManager) {
        const auto palette = m_themeManager->currentPalette();
        m_rippleColor = palette.rippleColor;
        connect(m_themeManager, &ThemeManager::themeChanged, this, [this]() {
            if (m_themeManager) {
                m_rippleColor = m_themeManager->currentPalette().rippleColor;
            }
            refreshForState(false);
        });
    }
}

QColor Button::normalBackground() const noexcept {
    return m_normalBackground;
}

void Button::setNormalBackground(const QColor& color) {
    if (!color.isValid() || color == m_normalBackground) {
        return;
    }
    m_normalBackground = color;
    refreshForState(false);
}

QColor Button::hoverBackground() const noexcept {
    return m_hoverBackground;
}

void Button::setHoverBackground(const QColor& color) {
    if (!color.isValid() || color == m_hoverBackground) {
        return;
    }
    m_hoverBackground = color;
    refreshForState(false);
}

QColor Button::pressedBackground() const noexcept {
    return m_pressedBackground;
}

void Button::setPressedBackground(const QColor& color) {
    if (!color.isValid() || color == m_pressedBackground) {
        return;
    }
    m_pressedBackground = color;
    refreshForState(false);
}

QColor Button::disabledBackground() const noexcept {
    return m_disabledBackground;
}

void Button::setDisabledBackground(const QColor& color) {
    if (!color.isValid() || color == m_disabledBackground) {
        return;
    }
    m_disabledBackground = color;
    refreshForState(false);
}

QColor Button::rippleColor() const noexcept {
    return m_rippleColor;
}

void Button::setRippleColor(const QColor& color) {
    if (!color.isValid() || color == m_rippleColor) {
        return;
    }
    m_rippleColor = color;
    update();
}

int Button::borderRadius() const noexcept {
    return m_borderRadius;
}

void Button::setBorderRadius(int radius) {
    m_borderRadius = clampInt(radius, 0, 24);
    update();
}

int Button::animationDuration() const noexcept {
    return m_animationDuration;
}

void Button::setAnimationDuration(int ms) {
    m_animationDuration = clampInt(ms, 100, 180);
}

void Button::applyPalette(const QColor& normal, const QColor& hover, const QColor& pressed,
                          const QColor& disabled) {
    if (normal.isValid()) {
        m_normalBackground = normal;
    }
    if (hover.isValid()) {
        m_hoverBackground = hover;
    }
    if (pressed.isValid()) {
        m_pressedBackground = pressed;
    }
    if (disabled.isValid()) {
        m_disabledBackground = disabled;
    }
    refreshForState(false);
}

void Button::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_currentBackground);

    const QRectF bgRect = QRectF(rect()).adjusted(0.5, 0.5, -0.5, -0.5);
    QPainterPath clipPath;
    clipPath.addRoundedRect(bgRect, static_cast<qreal>(m_borderRadius),
                            static_cast<qreal>(m_borderRadius));
    painter.drawPath(clipPath);

    if (m_rippleActive && isEnabled()) {
        const qreal radius = maxRippleRadius(m_rippleOrigin) * m_rippleProgress;
        if (radius > 0.5) {
            QColor ripple = m_rippleColor;
            const int alpha = static_cast<int>(
                static_cast<qreal>(m_rippleColor.alpha()) * (1.0 - m_rippleProgress));
            ripple.setAlpha(clampInt(alpha, 0, 255));
            painter.save();
            painter.setClipPath(clipPath);
            painter.setPen(Qt::NoPen);
            painter.setBrush(ripple);
            painter.drawEllipse(m_rippleOrigin, radius, radius);
            painter.restore();
        }
    }

    if (m_pressed && isEnabled()) {
        painter.setBrush(QColor(0, 0, 0, 28));
        painter.drawPath(clipPath);
    }

    if (m_loading) {
        painter.setPen(
            QPen(palette().color(QPalette::ButtonText), 2.5, Qt::SolidLine, Qt::RoundCap));
        painter.setBrush(Qt::NoPen);

        const int spinnerSize = 18;
        QRectF spinnerRect((width() - spinnerSize) / 2.0, (height() - spinnerSize) / 2.0,
                           spinnerSize, spinnerSize);

        painter.drawArc(spinnerRect, static_cast<int>(-m_loadingAngle * 16), 270 * 16);
    } else {
        QPushButton::paintEvent(event);
    }

    painter.end();
}

void Button::enterEvent(QEnterEvent* event) {
    QPushButton::enterEvent(event);
    m_hovered = true;
    if (isEnabled()) {
        startBackgroundAnimation(targetBackground());
    }
}

void Button::leaveEvent(QEvent* event) {
    QPushButton::leaveEvent(event);
    m_hovered = false;
    if (isEnabled()) {
        startBackgroundAnimation(targetBackground());
    }
}

void Button::mousePressEvent(QMouseEvent* event) {
    m_pressed = true;
    if (isEnabled()) {
        startBackgroundAnimation(m_pressedBackground);
        if (event->button() == Qt::LeftButton) {
            startRipple(event->position());
        }
    }
    QPushButton::mousePressEvent(event);
    update();
}

void Button::mouseReleaseEvent(QMouseEvent* event) {
    m_pressed = false;
    QPushButton::mouseReleaseEvent(event);
    if (isEnabled()) {
        startBackgroundAnimation(targetBackground());
    }
    update();
}

void Button::focusInEvent(QFocusEvent* event) {
    QPushButton::focusInEvent(event);
    update();
}

void Button::focusOutEvent(QFocusEvent* event) {
    QPushButton::focusOutEvent(event);
    update();
}

void Button::changeEvent(QEvent* event) {
    QPushButton::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        refreshForState(false);
    }
}

void Button::startBackgroundAnimation(const QColor& target) {
    if (!isEnabled()) {
        m_currentBackground = m_disabledBackground;
        update();
        return;
    }
    if (m_currentBackground == target && m_animation.isNull()) {
        update();
        return;
    }

    AnimationManager::stopAnimation(m_animation);

    const QColor from = m_currentBackground;
    m_animation = AnimationManager::animateColorValue(from, target, m_animationDuration, this,
                                                      [this](const QColor& color) {
                                                          m_currentBackground = color;
                                                          update();
                                                      });
}

void Button::refreshForState(bool animate) {
    const QColor target = targetBackground();
    if (!isEnabled() || !animate) {
        AnimationManager::stopAnimation(m_animation);
        m_currentBackground = target;
        update();
        return;
    }
    startBackgroundAnimation(target);
}

QColor Button::targetBackground() const noexcept {
    if (!isEnabled()) {
        return m_disabledBackground;
    }
    if (m_pressed) {
        return m_pressedBackground;
    }
    if (m_hovered) {
        return m_hoverBackground;
    }
    return m_normalBackground;
}

void Button::setLoading(bool loading) {
    if (m_loading == loading) {
        return;
    }
    m_loading = loading;
    if (m_loading) {
        m_loadingTimer->start();
        setEnabled(false);
    } else {
        m_loadingTimer->stop();
        setEnabled(true);
    }
    update();
}

bool Button::isLoading() const noexcept {
    return m_loading;
}

void Button::startRipple(const QPointF& origin) {
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

qreal Button::maxRippleRadius(const QPointF& origin) const noexcept {
    const qreal d1 = qHypot(origin.x(), origin.y());
    const qreal d2 = qHypot(width() - origin.x(), origin.y());
    const qreal d3 = qHypot(origin.x(), height() - origin.y());
    const qreal d4 = qHypot(width() - origin.x(), height() - origin.y());
    return qMax(qMax(d1, d2), qMax(d3, d4));
}
