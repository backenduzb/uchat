#pragma once

#include <QMap>
#include <QVector>

#include "chat/chat_repository.hpp"

class MockChatRepository final : public IChatRepository {
    Q_OBJECT

public:
    explicit MockChatRepository(QObject* parent = nullptr);

    [[nodiscard]] QVector<chat::Conversation> conversations() const override;
    [[nodiscard]] QVector<chat::Message> messages(
        const QString& conversationId) const override;
    [[nodiscard]] QVector<chat::Contact> contacts() const override;
    [[nodiscard]] std::optional<chat::Conversation> conversation(
        const QString& conversationId) const override;

    std::optional<chat::Message> sendMessage(
        const QString& conversationId, const QString& text) override;
    std::optional<chat::Conversation> createGroup(
        const QString& name, const QStringList& participantIds) override;
    void markAsRead(const QString& conversationId) override;

private:
    void seedData();
    void addConversation(
        const QString& id,
        const QString& name,
        chat::ConversationType type,
        const QStringList& participantIds,
        int unreadCount,
        const QDateTime& lastActivity,
        const QVector<std::pair<QString, QString>>& thread);
    void sortConversations();
    [[nodiscard]] int conversationIndex(const QString& conversationId) const;
    [[nodiscard]] QString nextMessageId();
    [[nodiscard]] QString nextConversationId();

    QVector<chat::Conversation> m_conversations;
    QMap<QString, QVector<chat::Message>> m_messages;
    QVector<chat::Contact> m_contacts;
    int m_messageCounter = 0;
    int m_conversationCounter = 0;
};
