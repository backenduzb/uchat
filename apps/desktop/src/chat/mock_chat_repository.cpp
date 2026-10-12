#include "mock_chat_repository.hpp"

#include <QDateTime>

#include <algorithm>
#include <utility>

namespace {

QDateTime minutesAgo(int minutes) {
    return QDateTime::currentDateTime().addSecs(-minutes * 60);
}

QDateTime hoursAgo(int hours) {
    return minutesAgo(hours * 60);
}

QDateTime daysAgo(int days) {
    return hoursAgo(days * 24);
}

} // namespace

MockChatRepository::MockChatRepository(QObject* parent) : IChatRepository(parent) {
    seedData();
}

QVector<chat::Conversation> MockChatRepository::conversations() const {
    return m_conversations;
}

QVector<chat::Message> MockChatRepository::messages(const QString& conversationId) const {
    return m_messages.value(conversationId);
}

QVector<chat::Contact> MockChatRepository::contacts() const {
    return m_contacts;
}

std::optional<chat::Conversation> MockChatRepository::conversation(
    const QString& conversationId) const {
    const int index = conversationIndex(conversationId);
    if (index < 0) {
        return std::nullopt;
    }
    return m_conversations.at(index);
}

std::optional<chat::Message> MockChatRepository::sendMessage(
    const QString& conversationId, const QString& text) {
    const QString trimmed = text.trimmed();
    const int index = conversationIndex(conversationId);
    if (index < 0 || trimmed.isEmpty()) {
        return std::nullopt;
    }
    chat::Message message;
    message.id = nextMessageId();
    message.conversationId = conversationId;
    message.senderId = QString::fromLatin1(chat::kLocalUserId);
    message.text = trimmed;
    message.timestamp = QDateTime::currentDateTime();

    m_messages[conversationId].push_back(message);

    chat::Conversation& conversation = m_conversations[index];
    conversation.preview = trimmed;
    conversation.lastActivity = message.timestamp;
    sortConversations();

    emit messagesChanged(conversationId);
    emit conversationsChanged();
    return message;
}

std::optional<chat::Conversation> MockChatRepository::createGroup(
    const QString& name, const QStringList& participantIds) {
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty() || participantIds.isEmpty()) {
        return std::nullopt;
    }
    chat::Conversation group;
    group.id = nextConversationId();
    group.name = trimmed;
    group.type = chat::ConversationType::Group;
    group.preview = QStringLiteral("Group created");
    group.lastActivity = QDateTime::currentDateTime();
    group.unreadCount = 0;
    group.participantIds = participantIds;
    group.participantIds.push_front(QString::fromLatin1(chat::kLocalUserId));

    m_conversations.push_front(group);
    m_messages.insert(group.id, {});

    emit conversationAdded(group.id);
    emit conversationsChanged();
    return group;
}

void MockChatRepository::markAsRead(const QString& conversationId) {
    const int index = conversationIndex(conversationId);
    if (index < 0 || m_conversations[index].unreadCount == 0) {
        return;
    }
    m_conversations[index].unreadCount = 0;
    emit conversationsChanged();
}

void MockChatRepository::seedData() {
    m_contacts = {
        {QStringLiteral("u-alice"), QStringLiteral("Alice Morgan")},
        {QStringLiteral("u-bob"), QStringLiteral("Bob Chen")},
        {QStringLiteral("u-carol"), QStringLiteral("Carol Diaz")},
        {QStringLiteral("u-david"), QStringLiteral("David Kim")},
        {QStringLiteral("u-eva"), QStringLiteral("Eva Rossi")},
        {QStringLiteral("u-frank"), QStringLiteral("Frank Miller")},
        {QStringLiteral("u-grace"), QStringLiteral("Grace Lee")},
        {QStringLiteral("u-hannah"), QStringLiteral("Hannah Cole")},
    };

    addConversation(QStringLiteral("c-alice"), QStringLiteral("Alice Morgan"),
                    chat::ConversationType::Private, {QStringLiteral("u-alice")}, 2,
                    minutesAgo(4),
                    {
                        {QStringLiteral("u-alice"), QStringLiteral("Hey! Are we still on for lunch tomorrow?")},
                        {QStringLiteral("me"), QStringLiteral("Yes! Noon at the usual place works for me.")},
                        {QStringLiteral("u-alice"), QStringLiteral("Perfect. I will book a table for two.")},
                        {QStringLiteral("u-alice"), QStringLiteral("Also, did you see the photos from the trip?")},
                    });

    addConversation(QStringLiteral("c-design"), QStringLiteral("Design Team"),
                    chat::ConversationType::Group,
                    {QStringLiteral("u-bob"), QStringLiteral("u-carol"), QStringLiteral("u-david")},
                    5, minutesAgo(12),
                    {
                        {QStringLiteral("u-carol"), QStringLiteral("The new onboarding mockups are ready for review.")},
                        {QStringLiteral("u-bob"), QStringLiteral("Nice work! The empty states look much cleaner now.")},
                        {QStringLiteral("me"), QStringLiteral("Agreed. Can we also check the dark mode contrast?")},
                        {QStringLiteral("u-david"), QStringLiteral("On it, I will post updated screenshots tonight.")},
                        {QStringLiteral("u-carol"), QStringLiteral("Reminder: design crit is at 3pm in the main room.")},
                    });

    addConversation(QStringLiteral("c-bob"), QStringLiteral("Bob Chen"),
                    chat::ConversationType::Private, {QStringLiteral("u-bob")}, 0,
                    minutesAgo(47),
                    {
                        {QStringLiteral("me"), QStringLiteral("Can you send me the build logs when you get a chance?")},
                        {QStringLiteral("u-bob"), QStringLiteral("Sure, uploading them now.")},
                        {QStringLiteral("u-bob"), QStringLiteral("Done. The failing test was a flaky timeout, rerunning fixed it.")},
                        {QStringLiteral("me"), QStringLiteral("Thanks! I will keep an eye on it this week.")},
                    });

    addConversation(QStringLiteral("c-hikers"), QStringLiteral("Weekend Hikers"),
                    chat::ConversationType::Group,
                    {QStringLiteral("u-eva"), QStringLiteral("u-frank"), QStringLiteral("u-grace"),
                     QStringLiteral("u-hannah"), QStringLiteral("u-alice")},
                    0, hoursAgo(2),
                    {
                        {QStringLiteral("u-eva"), QStringLiteral("Trail forecast for Saturday looks great, 18 degrees and clear.")},
                        {QStringLiteral("me"), QStringLiteral("Count me in. What time are we meeting?")},
                        {QStringLiteral("u-frank"), QStringLiteral("Lets meet at the north gate at 8am sharp.")},
                        {QStringLiteral("u-grace"), QStringLiteral("I will bring extra water and the first aid kit.")},
                        {QStringLiteral("u-eva"), QStringLiteral("Route map is pinned. See you all there!")},
                    });

    addConversation(QStringLiteral("c-carol"), QStringLiteral("Carol Diaz"),
                    chat::ConversationType::Private, {QStringLiteral("u-carol")}, 1,
                    hoursAgo(3),
                    {
                        {QStringLiteral("u-carol"), QStringLiteral("The client loved the proposal!")},
                        {QStringLiteral("me"), QStringLiteral("That is fantastic news, congratulations!")},
                        {QStringLiteral("u-carol"), QStringLiteral("They want a follow-up call next Tuesday to discuss timelines.")},
                    });

    addConversation(QStringLiteral("c-family"), QStringLiteral("Family"),
                    chat::ConversationType::Group,
                    {QStringLiteral("u-david"), QStringLiteral("u-eva"), QStringLiteral("u-grace"),
                     QStringLiteral("u-hannah")},
                    0, hoursAgo(6),
                    {
                        {QStringLiteral("u-hannah"), QStringLiteral("Sunday dinner at moms house, who is coming?")},
                        {QStringLiteral("me"), QStringLiteral("I will be there. Should I bring dessert?")},
                        {QStringLiteral("u-david"), QStringLiteral("Bring the apple pie, it was a hit last time.")},
                        {QStringLiteral("u-hannah"), QStringLiteral("Dinner is at 6. Dont be late this time!")},
                    });

    addConversation(QStringLiteral("c-david"), QStringLiteral("David Kim"),
                    chat::ConversationType::Private, {QStringLiteral("u-david")}, 0,
                    daysAgo(1),
                    {
                        {QStringLiteral("u-david"), QStringLiteral("Did you finish the code review?")},
                        {QStringLiteral("me"), QStringLiteral("Almost, left a few comments on the auth module.")},
                        {QStringLiteral("u-david"), QStringLiteral("Thanks, addressing them now.")},
                    });

    addConversation(QStringLiteral("c-eva"), QStringLiteral("Eva Rossi"),
                    chat::ConversationType::Private, {QStringLiteral("u-eva")}, 0,
                    daysAgo(2),
                    {
                        {QStringLiteral("me"), QStringLiteral("Happy birthday! Hope you have a wonderful day.")},
                        {QStringLiteral("u-eva"), QStringLiteral("Thank you so much! Dinner on Friday?")},
                        {QStringLiteral("me"), QStringLiteral("Would not miss it.")},
                    });

    addConversation(QStringLiteral("c-frank"), QStringLiteral("Frank Miller"),
                    chat::ConversationType::Private, {QStringLiteral("u-frank")}, 0,
                    daysAgo(3),
                    {
                        {QStringLiteral("u-frank"), QStringLiteral("The server migration is scheduled for tonight.")},
                        {QStringLiteral("me"), QStringLiteral("Good luck! Ping me if you need a second pair of eyes.")},
                    });

    addConversation(QStringLiteral("c-grace"), QStringLiteral("Grace Lee"),
                    chat::ConversationType::Private, {QStringLiteral("u-grace")}, 0,
                    daysAgo(5),
                    {
                        {QStringLiteral("u-grace"), QStringLiteral("Sharing the conference notes from last week.")},
                        {QStringLiteral("me"), QStringLiteral("Got them, very thorough. Thanks!")},
                    });

    sortConversations();
}

void MockChatRepository::addConversation(
    const QString& id,
    const QString& name,
    chat::ConversationType type,
    const QStringList& participantIds,
    int unreadCount,
    const QDateTime& lastActivity,
    const QVector<std::pair<QString, QString>>& thread) {
    chat::Conversation conversation;
    conversation.id = id;
    conversation.name = name;
    conversation.type = type;
    conversation.participantIds = participantIds;
    conversation.unreadCount = unreadCount;
    conversation.lastActivity = lastActivity;

    QVector<chat::Message> threadMessages;
    threadMessages.reserve(thread.size());
    const auto total = static_cast<int>(thread.size());
    for (int i = 0; i < total; ++i) {
        chat::Message message;
        message.id = nextMessageId();
        message.conversationId = id;
        message.senderId = thread.at(i).first;
        message.text = thread.at(i).second;
        const int offsetMinutes = 6 * (total - 1 - i);
        message.timestamp = lastActivity.addSecs(-offsetMinutes * 60);
        threadMessages.push_back(message);
    }
    if (!threadMessages.isEmpty()) {
        conversation.preview = threadMessages.constLast().text;
    }
    m_conversations.push_back(conversation);
    m_messages.insert(id, threadMessages);
}

void MockChatRepository::sortConversations() {
    std::sort(m_conversations.begin(), m_conversations.end(),
              [](const chat::Conversation& left, const chat::Conversation& right) {
                  return left.lastActivity > right.lastActivity;
              });
}

int MockChatRepository::conversationIndex(const QString& conversationId) const {
    for (qsizetype i = 0; i < m_conversations.size(); ++i) {
        if (m_conversations.at(i).id == conversationId) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

QString MockChatRepository::nextMessageId() {
    ++m_messageCounter;
    return QStringLiteral("m-%1").arg(m_messageCounter);
}

QString MockChatRepository::nextConversationId() {
    ++m_conversationCounter;
    return QStringLiteral("local-%1").arg(m_conversationCounter);
}
