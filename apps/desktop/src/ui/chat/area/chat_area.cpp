#include "ui/chat/area/chat_area.hpp"

#include <QLabel>
#include <QLineEdit>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>
#include <QStackedWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "chat/chat_repository.hpp"
#include "components/Button.hpp"
#include "components/IconButton.hpp"
#include "components/Lucide.hpp"
#include "theme/ThemeManager.hpp"
#include "ui/chat/message/bubble.hpp"

namespace {

constexpr int kHeaderAvatarSize = 40;

}

ChatArea::ChatArea(ThemeManager* themeManager, QWidget* parent)
    : QWidget(parent), m_themeManager(themeManager) {
    setObjectName("chatArea");
    setAttribute(Qt::WA_StyledBackground, true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    m_pages = new QStackedWidget(this);
    m_pages->setObjectName("chatPages");
    root->addWidget(m_pages, 1);

    m_emptyPage = new QWidget(m_pages);
    m_emptyPage->setObjectName("chatEmptyPage");
    auto* emptyLayout = new QVBoxLayout(m_emptyPage);
    emptyLayout->setContentsMargins(24, 24, 24, 24);
    emptyLayout->setSpacing(12);
    emptyLayout->addStretch(1);
    m_emptyIconLabel = new QLabel(m_emptyPage);
    m_emptyIconLabel->setObjectName("chatEmptyIcon");
    m_emptyIconLabel->setAlignment(Qt::AlignCenter);
    emptyLayout->addWidget(m_emptyIconLabel);
    m_emptyLabel = new QLabel(
        QStringLiteral("Select a conversation to start chatting."), m_emptyPage);
    m_emptyLabel->setObjectName("chatEmptyLabel");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setWordWrap(true);
    emptyLayout->addWidget(m_emptyLabel);
    emptyLayout->addStretch(1);
    m_pages->addWidget(m_emptyPage);

    m_chatPage = new QWidget(m_pages);
    m_chatPage->setObjectName("chatPage");
    auto* chatLayout = new QVBoxLayout(m_chatPage);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);

    auto* header = new QWidget(m_chatPage);
    header->setObjectName("chatHeader");
    header->setAttribute(Qt::WA_StyledBackground, true);
    auto* headerLayout = new QHBoxLayout(header);
    headerLayout->setContentsMargins(8, 8, 16, 8);
    headerLayout->setSpacing(8);

    m_backButton = new IconButton(Lucide::Icon::ArrowLeft, header, m_themeManager);
    m_backButton->setObjectName("chatBackButton");
    m_backButton->setFixedSize(40, 40);
    m_backButton->setIconSize(22);
    m_backButton->setToolTip(QStringLiteral("Back"));
    m_backButton->setAccessibleName(QStringLiteral("Back to conversations"));
    m_backButton->setVisible(false);
    connect(m_backButton, &QAbstractButton::clicked, this, &ChatArea::backRequested);
    headerLayout->addWidget(m_backButton, 0, Qt::AlignVCenter);

    m_avatarLabel = new QLabel(header);
    m_avatarLabel->setObjectName("chatAvatar");
    m_avatarLabel->setFixedSize(kHeaderAvatarSize, kHeaderAvatarSize);
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    QFont avatarFont = m_avatarLabel->font();
    avatarFont.setBold(true);
    avatarFont.setPointSize(12);
    m_avatarLabel->setFont(avatarFont);
    headerLayout->addWidget(m_avatarLabel, 0, Qt::AlignVCenter);

    auto* nameBox = new QVBoxLayout();
    nameBox->setContentsMargins(0, 0, 0, 0);
    nameBox->setSpacing(0);
    m_nameLabel = new QLabel(header);
    m_nameLabel->setObjectName("chatName");
    QFont nameFont = m_nameLabel->font();
    nameFont.setBold(true);
    nameFont.setPointSize(11);
    m_nameLabel->setFont(nameFont);
    m_subtitleLabel = new QLabel(header);
    m_subtitleLabel->setObjectName("chatSubtitle");
    nameBox->addWidget(m_nameLabel);
    nameBox->addWidget(m_subtitleLabel);
    headerLayout->addLayout(nameBox, 1);
    chatLayout->addWidget(header);

    auto* headerSeparator = new QWidget(m_chatPage);
    headerSeparator->setObjectName("chatSeparator");
    headerSeparator->setFixedHeight(1);
    headerSeparator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    chatLayout->addWidget(headerSeparator);

    m_messageScroll = new QScrollArea(m_chatPage);
    m_messageScroll->setObjectName("messageScroll");
    m_messageScroll->setWidgetResizable(true);
    m_messageScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_messageScroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_messageScroll->setFrameShape(QFrame::NoFrame);

    m_messageContent = new QWidget(m_messageScroll);
    m_messageContent->setObjectName("messageContent");
    m_messageLayout = new QVBoxLayout(m_messageContent);
    m_messageLayout->setContentsMargins(16, 12, 16, 12);
    m_messageLayout->setSpacing(2);
    m_noMessagesLabel = new QLabel(
        QStringLiteral("No messages yet. Say hello!"), m_messageContent);
    m_noMessagesLabel->setObjectName("noMessagesLabel");
    m_noMessagesLabel->setAlignment(Qt::AlignCenter);
    m_messageLayout->addWidget(m_noMessagesLabel);
    m_messageLayout->addStretch(1);
    m_messageScroll->setWidget(m_messageContent);
    chatLayout->addWidget(m_messageScroll, 1);

    auto* composer = new QWidget(m_chatPage);
    composer->setObjectName("composer");
    composer->setAttribute(Qt::WA_StyledBackground, true);
    auto* composerLayout = new QHBoxLayout(composer);
    composerLayout->setContentsMargins(16, 8, 16, 12);
    composerLayout->setSpacing(8);

    m_composerInput = new QLineEdit(composer);
    m_composerInput->setObjectName("composerInput");
    m_composerInput->setPlaceholderText(QStringLiteral("Write a message..."));
    connect(m_composerInput, &QLineEdit::returnPressed, this,
            &ChatArea::sendCurrentMessage);
    composerLayout->addWidget(m_composerInput, 1);

    m_sendButton = new Button(composer, m_themeManager);
    m_sendButton->setObjectName("sendButton");
    m_sendButton->setFixedSize(40, 40);
    m_sendButton->setBorderRadius(20);
    m_sendButton->setToolTip(QStringLiteral("Send"));
    m_sendButton->setAccessibleName(QStringLiteral("Send message"));
    connect(m_sendButton, &QPushButton::clicked, this,
            &ChatArea::sendCurrentMessage);
    composerLayout->addWidget(m_sendButton, 0, Qt::AlignVCenter);
    chatLayout->addWidget(composer);

    m_pages->addWidget(m_chatPage);
    m_pages->setCurrentWidget(m_emptyPage);

    if (m_themeManager) {
        connect(m_themeManager, &ThemeManager::appearanceChanged, this,
                &ChatArea::refreshTheme);
    }
    refreshTheme();
}

void ChatArea::setRepository(IChatRepository* repository) {
    if (m_repository) {
        disconnect(m_repository, nullptr, this, nullptr);
    }
    m_repository = repository;
    if (m_repository) {
        connect(m_repository, &IChatRepository::messagesChanged, this,
                &ChatArea::onMessagesChanged);
        connect(m_repository, &IChatRepository::conversationsChanged, this,
                &ChatArea::onConversationsChanged);
    }
}

QString ChatArea::currentConversationId() const {
    return m_currentId;
}

void ChatArea::setShowBackButton(bool show) {
    if (m_backButton) {
        m_backButton->setVisible(show);
    }
}

void ChatArea::setConversation(const QString& conversationId) {
    if (!m_repository) {
        return;
    }
    const auto conversation = m_repository->conversation(conversationId);
    if (!conversation.has_value()) {
        return;
    }
    if (!m_currentId.isEmpty() && m_messageScroll) {
        m_scrollPositions.insert(m_currentId, m_messageScroll->verticalScrollBar()->value());
    }
    const bool changed = (m_currentId != conversationId);
    m_currentId = conversationId;
    updateHeader();
    showChat();
    const int savedPosition = m_scrollPositions.value(m_currentId, -1);
    reloadMessages(savedPosition < 0);
    if (savedPosition >= 0 && m_messageScroll) {
        m_messageScroll->verticalScrollBar()->setValue(savedPosition);
    }
    if (changed) {
        m_repository->markAsRead(m_currentId);
    }
    m_composerInput->setFocus(Qt::OtherFocusReason);
}

void ChatArea::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    updateBubbleWidths();
}

void ChatArea::refreshTheme() {
    if (!m_themeManager) {
        return;
    }
    const ThemeManager::SurfaceColors surfaces = m_themeManager->surfaceColors();
    const QColor accent = m_themeManager->accentColor();
    const QColor topBar = m_themeManager->currentPalette().topBarBackground;

    setStyleSheet(QStringLiteral(
        "ChatArea#chatArea { background-color: %1; border: none; }"
        "QWidget#chatPage { background: transparent; border: none; }"
        "QWidget#chatHeader { background-color: %2; border: none; }"
        "QWidget#chatSeparator { background-color: %3; border: none; }"
        "QLabel#chatName { background: transparent; border: none; color: %4; }"
        "QLabel#chatSubtitle { background: transparent; border: none; color: %5; }"
        "QWidget#chatEmptyPage { background: transparent; border: none; }"
        "QLabel#chatEmptyLabel { background: transparent; border: none; color: %5; }"
        "QLabel#chatEmptyIcon { background: transparent; border: none; }"
        "QScrollArea#messageScroll { background: transparent; border: none; }"
        "QWidget#messageContent { background: transparent; border: none; }"
        "QLabel#noMessagesLabel { background: transparent; border: none; color: %5; }"
        "QWidget#composer { background: transparent; border: none; }"
        "QLineEdit#composerInput { background-color: %6; border: 1px solid transparent; "
        "border-radius: 20px; padding: 10px 16px; color: %4; "
        "selection-background-color: %7; }"
        "QLineEdit#composerInput:focus { border: 1px solid %7; }"
        "QScrollBar:vertical { background: transparent; width: 8px; margin: 2px; }"
        "QScrollBar::handle:vertical { background: %3; border-radius: 4px; min-height: 24px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }")
        .arg(surfaces.background.name(), topBar.name(), surfaces.stroke.name(),
             surfaces.foreground.name(), surfaces.muted.name(), surfaces.field.name(),
             accent.name()));

    const QColor emptyIcon = surfaces.muted;
    const qreal dpr = devicePixelRatioF();
    m_emptyIconLabel->setPixmap(
        Lucide::pixmap(Lucide::Icon::MessageCircle, 56, emptyIcon, 1.5, dpr));

    const QColor onAccent = ThemeManager::contrastText(accent);
    m_sendButton->setIcon(Lucide::icon(Lucide::Icon::Send, onAccent, 20));

    updateHeader();
    for (MessageBubble* bubble : m_bubbles) {
        if (bubble) {
            bubble->refreshTheme();
        }
    }
}

void ChatArea::reloadMessages(bool jumpToBottom) {
    if (!m_repository || m_currentId.isEmpty()) {
        return;
    }
    for (MessageBubble* bubble : m_bubbles) {
        m_messageLayout->removeWidget(bubble);
        bubble->deleteLater();
    }
    m_bubbles.clear();

    const QVector<chat::Message> loaded = m_repository->messages(m_currentId);
    m_noMessagesLabel->setVisible(loaded.isEmpty());
    int insertAt = 0;
    for (const auto& message : loaded) {
        auto* bubble = new MessageBubble(m_themeManager, message, m_messageContent);
        m_messageLayout->insertWidget(insertAt, bubble);
        m_bubbles.push_back(bubble);
        ++insertAt;
    }
    updateBubbleWidths();
    if (jumpToBottom) {
        scrollToBottom();
    }
}

void ChatArea::onMessagesChanged(const QString& conversationId) {
    if (conversationId != m_currentId || m_currentId.isEmpty()) {
        return;
    }
    const bool atBottom = m_messageScroll &&
        (m_messageScroll->verticalScrollBar()->value() >=
         m_messageScroll->verticalScrollBar()->maximum() - 40);
    reloadMessages(atBottom);
}

void ChatArea::onConversationsChanged() {
    if (!m_currentId.isEmpty()) {
        updateHeader();
    }
}

void ChatArea::sendCurrentMessage() {
    if (!m_repository || m_currentId.isEmpty()) {
        return;
    }
    const QString text = m_composerInput->text().trimmed();
    if (text.isEmpty()) {
        return;
    }
    if (m_repository->sendMessage(m_currentId, text).has_value()) {
        m_composerInput->clear();
    }
}

void ChatArea::showEmptyState() {
    m_pages->setCurrentWidget(m_emptyPage);
}

void ChatArea::showChat() {
    if (m_pages->currentWidget() != m_chatPage) {
        m_pages->setCurrentWidget(m_chatPage);
    }
}

void ChatArea::updateHeader() {
    if (!m_repository || m_currentId.isEmpty()) {
        return;
    }
    const auto conversation = m_repository->conversation(m_currentId);
    if (!conversation.has_value()) {
        showEmptyState();
        m_currentId.clear();
        return;
    }
    m_nameLabel->setText(conversation->name);
    m_subtitleLabel->setText(subtitleFor(m_currentId));
    const QColor avatarColor = chat::avatarColorFor(conversation->id);
    m_avatarLabel->setText(chat::contactInitials(conversation->name));
    const int radius = kHeaderAvatarSize / 2;
    m_avatarLabel->setStyleSheet(QStringLiteral(
        "QLabel#chatAvatar { background-color: %1; color: %2; border-radius: %3px; }")
        .arg(avatarColor.name(), chat::avatarTextColorFor(avatarColor).name())
        .arg(radius));
}

void ChatArea::updateBubbleWidths() {
    if (!m_messageScroll) {
        return;
    }
    const int viewportWidth = m_messageScroll->viewport()->width();
    const int maxWidth = static_cast<int>(static_cast<qreal>(viewportWidth) * 0.68);
    for (MessageBubble* bubble : m_bubbles) {
        if (bubble) {
            bubble->setMaxBubbleWidth(maxWidth > 120 ? maxWidth : 120);
        }
    }
}

void ChatArea::scrollToBottom() {
    if (!m_messageScroll) {
        return;
    }
    QTimer::singleShot(0, this, [this]() {
        if (m_messageScroll) {
            m_messageScroll->verticalScrollBar()->setValue(
                m_messageScroll->verticalScrollBar()->maximum());
        }
    });
}

QString ChatArea::subtitleFor(const QString& conversationId) const {
    if (!m_repository) {
        return {};
    }
    const auto conversation = m_repository->conversation(conversationId);
    if (!conversation.has_value()) {
        return {};
    }
    if (conversation->isGroup()) {
        const int count = static_cast<int>(conversation->participantIds.size());
        return tr("%n member(s)", "", count);
    }
    quint32 hash = 0;
    for (const QChar c : conversation->id) {
        hash = hash * 31 + static_cast<quint32>(c.unicode());
    }
    if (hash % 2 == 0) {
        return QStringLiteral("online");
    }
    return QStringLiteral("last seen recently");
}
