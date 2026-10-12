#pragma once

#include <QMap>
#include <QWidget>

class QLabel;
class QLineEdit;
class QScrollArea;
class QStackedWidget;
class QVBoxLayout;
class Button;
class IChatRepository;
class IconButton;
class MessageBubble;
class ThemeManager;

class ChatArea final : public QWidget {
    Q_OBJECT

public:
    explicit ChatArea(ThemeManager* themeManager, QWidget* parent = nullptr);

    void setRepository(IChatRepository* repository);
    [[nodiscard]] QString currentConversationId() const;
    void setShowBackButton(bool show);

signals:
    void backRequested();

public slots:
    void setConversation(const QString& conversationId);

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void refreshTheme();
    void reloadMessages(bool jumpToBottom);
    void onMessagesChanged(const QString& conversationId);
    void onConversationsChanged();
    void sendCurrentMessage();

private:
    void showEmptyState();
    void showChat();
    void updateHeader();
    void updateBubbleWidths();
    void scrollToBottom();
    [[nodiscard]] QString subtitleFor(const QString& conversationId) const;

    ThemeManager* m_themeManager = nullptr;
    IChatRepository* m_repository = nullptr;
    QString m_currentId;

    QStackedWidget* m_pages = nullptr;
    QWidget* m_emptyPage = nullptr;
    QWidget* m_chatPage = nullptr;

    QLabel* m_emptyIconLabel = nullptr;
    QLabel* m_emptyLabel = nullptr;

    QLabel* m_avatarLabel = nullptr;
    QLabel* m_nameLabel = nullptr;
    QLabel* m_subtitleLabel = nullptr;
    IconButton* m_backButton = nullptr;

    QScrollArea* m_messageScroll = nullptr;
    QWidget* m_messageContent = nullptr;
    QVBoxLayout* m_messageLayout = nullptr;
    QLabel* m_noMessagesLabel = nullptr;
    QVector<MessageBubble*> m_bubbles;

    QLineEdit* m_composerInput = nullptr;
    Button* m_sendButton = nullptr;

    QMap<QString, int> m_scrollPositions;
};
