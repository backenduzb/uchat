#pragma once

#include <QMap>
#include <QWidget>

#include "chat/chat_models.hpp"

class QLabel;
class QVBoxLayout;
class ConversationItemWidget;
class IChatRepository;
class ThemeManager;

class ConversationList final : public QWidget {
    Q_OBJECT

public:
    explicit ConversationList(
        ThemeManager* themeManager,
        QWidget* parent = nullptr);

    void setRepository(IChatRepository* repository);
    [[nodiscard]] QString selectedConversationId() const;
    void setSelectedConversation(const QString& conversationId);

public slots:
    void applyFilter(const QString& text);
    void openCreateGroup();

signals:
    void conversationSelected(const QString& conversationId);

private slots:
    void rebuildList();
    void onConversationAdded(const QString& conversationId);

private:
    void refreshTheme();
    void updateEmptyState();
    [[nodiscard]] bool matchesFilter(const chat::Conversation& conversation) const;

    ThemeManager* m_themeManager = nullptr;
    IChatRepository* m_repository = nullptr;

    QVBoxLayout* m_itemsLayout = nullptr;
    QLabel* m_emptyLabel = nullptr;
    QWidget* m_listContent = nullptr;

    QMap<QString, ConversationItemWidget*> m_items;
    QString m_selectedId;
    QString m_filter;
};
