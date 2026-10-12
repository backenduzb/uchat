#include "InputField.hpp"

#include <QFocusEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QVariantAnimation>

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

InputField::InputField(QWidget* parent, ThemeManager* themeManager)
    : QLineEdit(parent), m_themeManager(themeManager) {
    setFrame(false);

    if (m_themeManager) {
        const auto palette = m_themeManager->currentPalette();
        m_normalBorderColor = palette.inputBorderNormal;
        m_focusedBorderColor = palette.inputBorderFocused;
        m_disabledBorderColor = palette.inputBorderDisabled;
        m_placeholderColor = palette.inputPlaceholder;
        setThemeManager(m_themeManager);
    }

    setPlaceholderColor(m_placeholderColor);
    refreshCurrentColor();
}

void InputField::setThemeManager(ThemeManager* themeManager) {
    if (m_themeManager) {
        disconnect(m_themeManager, nullptr, this, nullptr);
    }
    m_themeManager = themeManager;
    if (m_themeManager) {
        connect(
            m_themeManager, &ThemeManager::themeChanged, this,
            [this]() { refreshCurrentColor(); });
    }
}

QColor InputField::normalBorderColor() const noexcept {
    return m_normalBorderColor;
}

void InputField::setNormalBorderColor(const QColor& color) {
    if (!color.isValid() || color == m_normalBorderColor) {
        return;
    }
    m_normalBorderColor = color;
    refreshCurrentColor();
}

QColor InputField::focusedBorderColor() const noexcept {
    return m_focusedBorderColor;
}

void InputField::setFocusedBorderColor(const QColor& color) {
    if (!color.isValid() || color == m_focusedBorderColor) {
        return;
    }
    m_focusedBorderColor = color;
    refreshCurrentColor();
}

QColor InputField::disabledBorderColor() const noexcept {
    return m_disabledBorderColor;
}

void InputField::setDisabledBorderColor(const QColor& color) {
    if (!color.isValid() || color == m_disabledBorderColor) {
        return;
    }
    m_disabledBorderColor = color;
    refreshCurrentColor();
}

QColor InputField::placeholderColor() const noexcept {
    return m_placeholderColor;
}

void InputField::setPlaceholderColor(const QColor& color) {
    if (!color.isValid()) {
        return;
    }
    m_placeholderColor = color;
    QPalette pal = palette();
    pal.setColor(QPalette::PlaceholderText, m_placeholderColor);
    setPalette(pal);
}

int InputField::underlineHeight() const noexcept {
    return m_underlineHeight;
}

void InputField::setUnderlineHeight(int height) {
    m_underlineHeight = clampInt(height, 1, 4);
    update();
}

int InputField::focusedUnderlineHeight() const noexcept {
    return m_focusedUnderlineHeight;
}

void InputField::setFocusedUnderlineHeight(int height) {
    m_focusedUnderlineHeight = clampInt(height, 1, 4);
    update();
}

int InputField::animationDuration() const noexcept {
    return m_animationDuration;
}

void InputField::setAnimationDuration(int ms) {
    m_animationDuration = clampInt(ms, 100, 180);
}

void InputField::applyPalette(
    const QColor& normal,
    const QColor& focused,
    const QColor& disabled,
    const QColor& placeholder) {
    if (normal.isValid()) {
        m_normalBorderColor = normal;
    }
    if (focused.isValid()) {
        m_focusedBorderColor = focused;
    }
    if (disabled.isValid()) {
        m_disabledBorderColor = disabled;
    }
    if (placeholder.isValid()) {
        setPlaceholderColor(placeholder);
    }
    refreshCurrentColor();
}

void InputField::paintEvent(QPaintEvent* event) {
    QLineEdit::paintEvent(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);

    const int baseH = m_underlineHeight;
    const int focusH = m_focusedUnderlineHeight;
    const int thickness = baseH + static_cast<int>(
        static_cast<double>(focusH - baseH) * m_focusProgress);

    painter.fillRect(0, height() - thickness, width(), thickness, m_currentBorderColor);
}

void InputField::focusInEvent(QFocusEvent* event) {
    QLineEdit::focusInEvent(event);
    startFocusAnimation(true);
}

void InputField::focusOutEvent(QFocusEvent* event) {
    QLineEdit::focusOutEvent(event);
    startFocusAnimation(false);
}

void InputField::changeEvent(QEvent* event) {
    QLineEdit::changeEvent(event);
    if (event->type() == QEvent::EnabledChange) {
        AnimationManager::stopAnimation(m_animation);
        m_focusProgress = hasFocus() && isEnabled() ? 1.0 : 0.0;
        refreshCurrentColor();
    }
}

void InputField::startFocusAnimation(bool focused) {
    if (!isEnabled()) {
        return;
    }
    const double target = focused ? 1.0 : 0.0;
    if (m_focusProgress == target && m_animation.isNull()) {
        refreshCurrentColor();
        return;
    }

    AnimationManager::stopAnimation(m_animation);

    const double from = m_focusProgress;

    m_animation = AnimationManager::animateProgress(
        from, target, m_animationDuration, this,
        [this](double progress) {
            m_focusProgress = progress;
            if (isEnabled()) {
                m_currentBorderColor = blendColors(
                    m_normalBorderColor, m_focusedBorderColor, m_focusProgress);
            } else {
                m_currentBorderColor = m_disabledBorderColor;
            }
            update();
        });
}

void InputField::refreshCurrentColor() {
    if (!isEnabled()) {
        m_currentBorderColor = m_disabledBorderColor;
    } else {
        m_currentBorderColor = blendColors(
            m_normalBorderColor, m_focusedBorderColor, m_focusProgress);
    }
    update();
}

QColor InputField::targetBorderColor() const noexcept {
    if (!isEnabled()) {
        return m_disabledBorderColor;
    }
    return hasFocus() ? m_focusedBorderColor : m_normalBorderColor;
}

QColor InputField::blendColors(const QColor& from, const QColor& to, double t) noexcept {
    const double clamped = t < 0.0 ? 0.0 : (t > 1.0 ? 1.0 : t);
    const auto lerp = [clamped](int a, int b) {
        return static_cast<int>(static_cast<double>(a) +
            (static_cast<double>(b - a) * clamped));
    };
    return QColor(
        lerp(from.red(), to.red()),
        lerp(from.green(), to.green()),
        lerp(from.blue(), to.blue()),
        lerp(from.alpha(), to.alpha()));
}
