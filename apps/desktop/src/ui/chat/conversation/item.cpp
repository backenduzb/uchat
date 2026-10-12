#include "ui/chat/conversation/item.hpp"

#include <QEnterEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QVBoxLayout>
#include <QtMath>

#include "animation/AnimationManager.hpp"
#include "theme/ThemeManager.hpp"

namespace {

constexpr int kItemHeight = 72;
constexpr int kAvatarSize = 48;

QColor blendOver(const QColor& top, const QColor& base, qreal alpha) {
    const qreal clamped = alpha < 0.0 ? 0.0 : (alpha > 1.0 ? 1.0 : alpha);
    const auto mix = [clamped](int topChannel, int baseChannel) {
        return static_cast<int>(static_cast<qreal>(topChannel) * clamped +
            static_cast<qreal>(baseChannel) * (1.0 - clamped));
    };
    return QColor(mix(top.red(), base.red()), mix(top.green(), base.green()),
                  mix(top.blue(), base.blue()));
}

}

ConversationItemWidget::ConversationItemWidget(
    ThemeManager* themeManager,
    const chat::Conversation& conversation,
    QWidget* parent)
    : QWidget(parent), m_themeManager(themeManager), m_conversation(conversation) {
    setObjectName("conversationItem");
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedHeight(kItemHeight);
    setCursor(Qt::PointingHandCursor);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 8, 12, 8);
    layout->setSpacing(12);

    m_avatarLabel = new QLabel(this);
    m_avatarLabel->setObjectName("itemAvatar");
    m_avatarLabel->setFixedSize(kAvatarSize, kAvatarSize);
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    QFont avatarFont = m_avatarLabel->font();
    avatarFont.setBold(true);
    avatarFont.setPointSize(13);
    m_avatarLabel->setFont(avatarFont);
    layout->addWidget(m_avatarLabel, 0, Qt::AlignVCenter);

    auto* textBox = new QVBoxLayout();
    textBox->setContentsMargins(0, 0, 0, 0);
    textBox->setSpacing(2);
    m_nameLabel = new QLabel(this);
    m_nameLabel->setObjectName("itemName");
    QFont nameFont = m_nameLabel->font();
    nameFont.setBold(true);
    nameFont.setPointSize(10);
    m_nameLabel->setFont(nameFont);
    m_previewLabel = new QLabel(this);
    m_previewLabel->setObjectName("itemPreview");
    textBox->addWidget(m_nameLabel);
    textBox->addWidget(m_previewLabel);
    layout->addLayout(textBox, 1);

    auto* metaBox = new QVBoxLayout();
    metaBox->setContentsMargins(0, 0, 0, 0);
    metaBox->setSpacing(4);
    m_timeLabel = new QLabel(this);
    m_timeLabel->setObjectName("itemTime");
    m_badgeLabel = new QLabel(this);
    m_badgeLabel->setObjectName("itemBadge");
    m_badgeLabel->setAlignment(Qt::AlignCenter);
    m_badgeLabel->setFixedHeight(20);
    m_badgeLabel->setMinimumWidth(20);
    metaBox->addWidget(m_timeLabel, 0, Qt::AlignRight);
    metaBox->addWidget(m_badgeLabel, 0, Qt::AlignRight);
    layout->addLayout(metaBox, 0);

    updateStateColors();
    m_currentBackground = targetBackground();
    updateConversation(conversation);
    if (m_themeManager) {
        connect(m_themeManager, &ThemeManager::appearanceChanged, this,
                &ConversationItemWidget::refreshTheme);
    }
    refreshTheme();
}

QString ConversationItemWidget::conversationId() const {
    return m_conversation.id;
}

void ConversationItemWidget::updateConversation(const chat::Conversation& conversation) {
    m_conversation = conversation;
    m_nameLabel->setText(conversation.name);
    const QString preview = conversation.preview.trimmed();
    m_previewLabel->setText(preview.isEmpty() ? QStringLiteral("No messages yet") : preview);
    m_timeLabel->setText(chat::formatListTime(conversation.lastActivity));
    if (conversation.unreadCount > 0) {
        m_badgeLabel->setText(conversation.unreadCount > 99
                                  ? QStringLiteral("99+")
                                  : QString::number(conversation.unreadCount));
        m_badgeLabel->setVisible(true);
    } else {
        m_badgeLabel->setVisible(false);
    }
    QFont nameFont = m_nameLabel->font();
    nameFont.setBold(m_selected || conversation.unreadCount > 0);
    m_nameLabel->setFont(nameFont);
    refreshBackground(false);
}

void ConversationItemWidget::setSelected(bool selected) {
    if (m_selected == selected) {
        return;
    }
    m_selected = selected;
    QFont nameFont = m_nameLabel->font();
    nameFont.setBold(m_selected || m_conversation.unreadCount > 0);
    m_nameLabel->setFont(nameFont);
    refreshBackground(true);
}

void ConversationItemWidget::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_currentBackground);
    painter.drawRect(rect());

    if (m_rippleActive) {
        const qreal radius = maxRippleRadius(m_rippleOrigin) * m_rippleProgress;
        if (radius > 0.5) {
            QColor ripple = m_rippleColor;
            const int alpha = static_cast<int>(
                static_cast<qreal>(m_rippleColor.alpha()) * (1.0 - m_rippleProgress));
            const int clamped = alpha < 0 ? 0 : (alpha > 255 ? 255 : alpha);
            ripple.setAlpha(clamped);
            painter.setBrush(ripple);
            painter.setClipRect(rect());
            painter.drawEllipse(m_rippleOrigin, radius, radius);
            painter.setClipping(false);
        }
    }
}

void ConversationItemWidget::mousePressEvent(QMouseEvent* event) {
    m_pressed = true;
    startBackgroundAnimation(targetBackground());
    if (event->button() == Qt::LeftButton) {
        startRipple(event->position());
        event->accept();
        emit pressed(m_conversation.id);
        return;
    }
    QWidget::mousePressEvent(event);
}

void ConversationItemWidget::mouseReleaseEvent(QMouseEvent* event) {
    m_pressed = false;
    QWidget::mouseReleaseEvent(event);
    startBackgroundAnimation(targetBackground());
    update();
}

void ConversationItemWidget::enterEvent(QEnterEvent* event) {
    QWidget::enterEvent(event);
    m_hovered = true;
    startBackgroundAnimation(targetBackground());
}

void ConversationItemWidget::leaveEvent(QEvent* event) {
    QWidget::leaveEvent(event);
    m_hovered = false;
    m_pressed = false;
    startBackgroundAnimation(targetBackground());
}

void ConversationItemWidget::refreshTheme() {
    const QColor avatarColor = chat::avatarColorFor(m_conversation.id);
    m_avatarLabel->setText(chat::contactInitials(m_conversation.name));
    m_avatarLabel->setStyleSheet(QStringLiteral(
        "QLabel#itemAvatar { background-color: %1; color: %2; border-radius: 24px; }")
        .arg(avatarColor.name(), chat::avatarTextColorFor(avatarColor).name()));
    if (m_themeManager) {
        m_rippleColor = m_themeManager->currentPalette().iconRippleColor;
    }
    updateStateColors();
    restyleLabels();
    refreshBackground(false);
}

void ConversationItemWidget::updateStateColors() {
    QColor field("#F0F0F0");
    QColor accent("#229ED9");
    if (m_themeManager) {
        field = m_themeManager->surfaceColors().field;
        accent = m_themeManager->accentColor();
    }
    m_normalBackground = field;
    m_hoverBackground = blendOver(accent, field, 0.08);
    m_pressedBackground = blendOver(accent, field, 0.32);
    m_selectedBackground = blendOver(accent, field, 0.22);
}

void ConversationItemWidget::restyleLabels() {
    QString foreground = QStringLiteral("#000000");
    QString muted = QStringLiteral("#767676");
    QString badgeBg = QStringLiteral("#FF5C5C");
    if (m_themeManager) {
        const ThemeManager::SurfaceColors surfaces = m_themeManager->surfaceColors();
        foreground = surfaces.foreground.name();
        muted = surfaces.muted.name();
        badgeBg = m_themeManager->currentPalette().badgeColor.name();
    }
    setStyleSheet(QStringLiteral(
        "ConversationItemWidget#conversationItem { background: transparent; "
        "border: none; border-radius: 0px; }"
        "QLabel#itemName { background: transparent; border: none; color: %1; }"
        "QLabel#itemPreview { background: transparent; border: none; color: %2; }"
        "QLabel#itemTime { background: transparent; border: none; color: %2; }"
        "QLabel#itemBadge { background-color: %3; color: #FFFFFF; border: none; "
        "border-radius: 10px; padding: 0px 6px; }")
        .arg(foreground, muted, badgeBg));
}

void ConversationItemWidget::refreshBackground(bool animate) {
    const QColor target = targetBackground();
    if (!animate) {
        AnimationManager::stopAnimation(m_animation);
        m_currentBackground = target;
        update();
        return;
    }
    startBackgroundAnimation(target);
}

void ConversationItemWidget::startBackgroundAnimation(const QColor& target) {
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

void ConversationItemWidget::startRipple(const QPointF& origin) {
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

QColor ConversationItemWidget::targetBackground() const noexcept {
    if (m_pressed) {
        return m_pressedBackground;
    }
    if (m_selected) {
        return m_selectedBackground;
    }
    if (m_hovered) {
        return m_hoverBackground;
    }
    return m_normalBackground;
}

qreal ConversationItemWidget::maxRippleRadius(const QPointF& origin) const noexcept {
    const qreal d1 = qHypot(origin.x(), origin.y());
    const qreal d2 = qHypot(width() - origin.x(), origin.y());
    const qreal d3 = qHypot(origin.x(), height() - origin.y());
    const qreal d4 = qHypot(width() - origin.x(), height() - origin.y());
    return qMax(qMax(d1, d2), qMax(d3, d4));
}
