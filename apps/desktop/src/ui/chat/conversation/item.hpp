#pragma once

#include <QColor>
#include <QPointer>
#include <QWidget>

#include "chat/chat_models.hpp"

class QLabel;
class QVariantAnimation;
class ThemeManager;

class ConversationItemWidget final : public QWidget {
    Q_OBJECT

public:
    explicit ConversationItemWidget(
        ThemeManager* themeManager,
        const chat::Conversation& conversation,
        QWidget* parent = nullptr);

    [[nodiscard]] QString conversationId() const;
    void updateConversation(const chat::Conversation& conversation);
    void setSelected(bool selected);

signals:
    void pressed(const QString& conversationId);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private slots:
    void refreshTheme();

private:
    void updateStateColors();
    void restyleLabels();
    void refreshBackground(bool animate);
    void startBackgroundAnimation(const QColor& target);
    void startRipple(const QPointF& origin);
    [[nodiscard]] QColor targetBackground() const noexcept;
    [[nodiscard]] qreal maxRippleRadius(const QPointF& origin) const noexcept;

    ThemeManager* m_themeManager = nullptr;
    chat::Conversation m_conversation;
    bool m_selected = false;
    bool m_hovered = false;
    bool m_pressed = false;

    QColor m_normalBackground = QColor("#F0F0F0");
    QColor m_hoverBackground = QColor("#F0F0F0");
    QColor m_pressedBackground = QColor("#F0F0F0");
    QColor m_selectedBackground = QColor("#F0F0F0");
    QColor m_currentBackground = QColor("#F0F0F0");
    QColor m_rippleColor = QColor(0, 0, 0, 40);
    QPointF m_rippleOrigin;
    double m_rippleProgress = 1.0;
    bool m_rippleActive = false;
    int m_rippleDuration = 450;
    int m_animationDuration = 150;

    QPointer<QVariantAnimation> m_animation;
    QPointer<QVariantAnimation> m_rippleAnimation;

    QLabel* m_avatarLabel = nullptr;
    QLabel* m_nameLabel = nullptr;
    QLabel* m_previewLabel = nullptr;
    QLabel* m_timeLabel = nullptr;
    QLabel* m_badgeLabel = nullptr;
};
