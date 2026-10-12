#pragma once

#include <QColor>
#include <QPointer>
#include <QPushButton>
#include <qtypes.h>

class QVariantAnimation;
class ThemeManager;

class Button final : public QPushButton {
    Q_OBJECT

    Q_PROPERTY(QColor normalBackground READ normalBackground WRITE setNormalBackground)
    Q_PROPERTY(QColor hoverBackground READ hoverBackground WRITE setHoverBackground)
    Q_PROPERTY(QColor pressedBackground READ pressedBackground WRITE setPressedBackground)
    Q_PROPERTY(QColor disabledBackground READ disabledBackground WRITE setDisabledBackground)
    Q_PROPERTY(QColor rippleColor READ rippleColor WRITE setRippleColor)
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius)
    Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration)

public:
    explicit Button(QWidget* parent = nullptr, ThemeManager* themeManager = nullptr);
    explicit Button(const QString& text, QWidget* parent = nullptr,
                    ThemeManager* themeManager = nullptr);

    void setThemeManager(ThemeManager* themeManager);

    [[nodiscard]] QColor normalBackground() const noexcept;
    void setNormalBackground(const QColor& color);

    [[nodiscard]] QColor hoverBackground() const noexcept;
    void setHoverBackground(const QColor& color);

    [[nodiscard]] QColor pressedBackground() const noexcept;
    void setPressedBackground(const QColor& color);

    [[nodiscard]] QColor disabledBackground() const noexcept;
    void setDisabledBackground(const QColor& color);

    [[nodiscard]] QColor rippleColor() const noexcept;
    void setRippleColor(const QColor& color);

    [[nodiscard]] int borderRadius() const noexcept;
    void setBorderRadius(int radius);

    [[nodiscard]] int animationDuration() const noexcept;
    void setAnimationDuration(int ms);

    void applyPalette(
        const QColor& normal,
        const QColor& hover,
        const QColor& pressed,
        const QColor& disabled);
    void setLoading(bool loading);
    bool isLoading() const noexcept;
    
protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void startBackgroundAnimation(const QColor& target);
    void refreshForState(bool animate);
    [[nodiscard]] QColor targetBackground() const noexcept;

    QColor m_normalBackground = QColor("#000000");
    QColor m_hoverBackground = QColor("#333333");
    QColor m_pressedBackground = QColor("#555555");
    QColor m_disabledBackground = QColor("#E5E5E5");
    QColor m_currentBackground = QColor("#000000");
    QColor m_rippleColor = QColor(255, 255, 255, 70);
    QPointF m_rippleOrigin;
    double m_rippleProgress = 1.0;
    bool m_rippleActive = false;
    int m_rippleDuration = 450;
    bool m_loading = false;
    QTimer *m_loadingTimer = nullptr;
    qreal m_loadingAngle = 0.0;
    int m_borderRadius = 10;
    int m_animationDuration = 150;

    bool m_hovered = false;
    bool m_pressed = false;

    QPointer<QVariantAnimation> m_animation;
    QPointer<QVariantAnimation> m_rippleAnimation;
    ThemeManager* m_themeManager = nullptr;
    void startRipple(const QPointF& origin);
    [[nodiscard]] qreal maxRippleRadius(const QPointF& origin) const noexcept;
};
