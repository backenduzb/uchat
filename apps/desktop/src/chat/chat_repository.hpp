#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

#include <optional>

#include "chat/chat_models.hpp"

class IChatRepository : public QObject {
    Q_OBJECT

public:
    explicit IChatRepository(QObject* parent = nullptr) : QObject(parent) {}
    ~IChatRepository() override = default;

    IChatRepository(const IChatRepository&) = delete;
    IChatRepository& operator=(const IChatRepository&) = delete;

    [[nodiscard]] virtual QVector<chat::Conversation> conversations() const = 0;
    [[nodiscard]] virtual QVector<chat::Message> messages(
        const QString& conversationId) const = 0;
    [[nodiscard]] virtual QVector<chat::Contact> contacts() const = 0;
    [[nodiscard]] virtual std::optional<chat::Conversation> conversation(
        const QString& conversationId) const = 0;

    virtual std::optional<chat::Message> sendMessage(
        const QString& conversationId, const QString& text) = 0;

    virtual std::optional<chat::Conversation> createGroup(
        const QString& name, const QStringList& participantIds) = 0;

    virtual void markAsRead(const QString& conversationId) = 0;

signals:
    void conversationsChanged();
    void messagesChanged(const QString& conversationId);
    void conversationAdded(const QString& conversationId);
};
