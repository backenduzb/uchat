#pragma once

#include <QAbstractButton>
#include <QColor>
#include <QPointer>

#include "components/Lucide.hpp"

class QVariantAnimation;
class ThemeManager;

class IconButton final : public QAbstractButton {
    Q_OBJECT

    Q_PROPERTY(QColor iconColor READ iconColor WRITE setIconColor)
    Q_PROPERTY(QColor rippleColor READ rippleColor WRITE setRippleColor)
    Q_PROPERTY(int iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration)

public:
    explicit IconButton(QWidget* parent = nullptr, ThemeManager* themeManager = nullptr);
    explicit IconButton(Lucide::Icon icon, QWidget* parent = nullptr,
                        ThemeManager* themeManager = nullptr);

    void setThemeManager(ThemeManager* themeManager);

    void setIcon(Lucide::Icon icon);
    [[nodiscard]] Lucide::Icon icon() const noexcept;

    [[nodiscard]] QColor iconColor() const noexcept;
    void setIconColor(const QColor& color);

    [[nodiscard]] QColor rippleColor() const noexcept;
    void setRippleColor(const QColor& color);

    [[nodiscard]] int iconSize() const noexcept;
    void setIconSize(int size);

    [[nodiscard]] int animationDuration() const noexcept;
    void setAnimationDuration(int ms);

    void setShowBadge(bool show);
    [[nodiscard]] bool showBadge() const noexcept;
    void setBadgeColor(const QColor& color);

    [[nodiscard]] QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    void startRipple(const QPointF& origin);
    void startIconAnimation(const QColor& target);
    void refreshFromTheme(bool animate);
    [[nodiscard]] QColor targetIconColor() const noexcept;
    [[nodiscard]] qreal maxRippleRadius(const QPointF& origin) const noexcept;

    Lucide::Icon m_icon = Lucide::Icon::Menu;
    QColor m_iconColor = QColor("#F5F5F5");
    QColor m_iconHoverColor = QColor("#FFFFFF");
    QColor m_currentIconColor = QColor("#F5F5F5");
    QColor m_rippleColor = QColor(255, 255, 255, 60);
    QColor m_badgeColor = QColor("#FF5C5C");

    int m_iconSize = 22;
    int m_animationDuration = 220;
    int m_rippleDuration = 450;

    bool m_hovered = false;
    bool m_showBadge = false;

    QPointF m_rippleOrigin;
    double m_rippleProgress = 1.0;
    bool m_rippleActive = false;

    QPointer<QVariantAnimation> m_rippleAnimation;
    QPointer<QVariantAnimation> m_iconAnimation;
    ThemeManager* m_themeManager = nullptr;
};
