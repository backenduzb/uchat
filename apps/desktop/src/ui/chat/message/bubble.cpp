#include "ui/chat/message/bubble.hpp"

#include <QHBoxLayout>
#include <QLabel>

#include "theme/ThemeManager.hpp"

namespace {

constexpr int kBubbleHPadding = 10;
constexpr int kBubbleVPadding = 6;
constexpr int kTimeWidth = 44;

}

MessageBubble::MessageBubble(
    ThemeManager* themeManager,
    const chat::Message& message,
    QWidget* parent)
    : QWidget(parent), m_themeManager(themeManager), m_message(message) {
    setObjectName("messageBubble");
    setAttribute(Qt::WA_StyledBackground, true);

    auto* row = new QHBoxLayout(this);
    row->setContentsMargins(0, 1, 0, 1);
    row->setSpacing(0);

    m_bubble = new QWidget(this);
    m_bubble->setObjectName(message.isOutgoing() ? "bubbleOutgoing" : "bubbleIncoming");
    m_bubble->setAttribute(Qt::WA_StyledBackground, true);
    m_bubble->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);

    auto* bubbleLayout = new QHBoxLayout(m_bubble);
    bubbleLayout->setContentsMargins(
        kBubbleHPadding, kBubbleVPadding, kBubbleHPadding, kBubbleVPadding);
    bubbleLayout->setSpacing(8);

    m_textLabel = new QLabel(message.text, m_bubble);
    m_textLabel->setObjectName("bubbleText");
    m_textLabel->setWordWrap(true);
    m_textLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_textLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    bubbleLayout->addWidget(m_textLabel, 1);

    m_timeLabel = new QLabel(chat::formatMessageTime(message.timestamp), m_bubble);
    m_timeLabel->setObjectName("bubbleTime");
    m_timeLabel->setFixedWidth(kTimeWidth);
    m_timeLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    m_timeLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::MinimumExpanding);
    bubbleLayout->addWidget(m_timeLabel, 0, Qt::AlignBottom);

    if (message.isOutgoing()) {
        row->addStretch(1);
        row->addWidget(m_bubble, 0, Qt::AlignRight);
    } else {
        row->addWidget(m_bubble, 0, Qt::AlignLeft);
        row->addStretch(1);
    }

    if (m_themeManager) {
        connect(m_themeManager, &ThemeManager::appearanceChanged, this,
                &MessageBubble::refreshTheme);
    }
    refreshTheme();
}

void MessageBubble::setMaxBubbleWidth(int width) {
    if (width > 0) {
        m_textLabel->setMaximumWidth(width - kBubbleHPadding * 2 - 8 - kTimeWidth);
    }
}

void MessageBubble::refreshTheme() {
    if (!m_themeManager) {
        return;
    }
    const ThemeManager::SurfaceColors surfaces = m_themeManager->surfaceColors();
    const QColor accent = m_themeManager->accentColor();
    const QColor onAccent = ThemeManager::contrastText(accent);

    const QString bubbleBg =
        m_message.isOutgoing() ? accent.name() : surfaces.field.name();
    const QString textColor =
        m_message.isOutgoing() ? onAccent.name() : surfaces.foreground.name();
    const QString timeColor =
        m_message.isOutgoing() ? onAccent.name() : surfaces.muted.name();
    const QString cornerRadii = m_message.isOutgoing()
        ? QStringLiteral("border-top-right-radius: 4px;")
        : QStringLiteral("border-top-left-radius: 4px;");

    m_bubble->setStyleSheet(QStringLiteral(
        "QWidget#%1 { background-color: %2; border: none; border-radius: 12px; %3 }")
        .arg(m_bubble->objectName(), bubbleBg, cornerRadii));
    m_textLabel->setStyleSheet(QStringLiteral(
        "QLabel#bubbleText { background: transparent; border: none; color: %1; }")
        .arg(textColor));
    QFont timeFont = m_timeLabel->font();
    timeFont.setPointSize(8);
    m_timeLabel->setFont(timeFont);
    m_timeLabel->setStyleSheet(QStringLiteral(
        "QLabel#bubbleTime { background: transparent; border: none; color: %1; }")
        .arg(timeColor));
    setStyleSheet(QStringLiteral(
        "MessageBubble#messageBubble { background: transparent; border: none; }"));
}
