#include "ui/chat/conversation/list.hpp"

#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

#include "chat/chat_repository.hpp"
#include "theme/ThemeManager.hpp"
#include "ui/chat/conversation/item.hpp"
#include "ui/chat/group/dialog.hpp"

ConversationList::ConversationList(ThemeManager* themeManager, QWidget* parent)
    : QWidget(parent), m_themeManager(themeManager) {
    setObjectName("conversationList");
    setAttribute(Qt::WA_StyledBackground, true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    auto* scroll = new QScrollArea(this);
    scroll->setObjectName("listScroll");
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setFrameShape(QFrame::NoFrame);

    m_listContent = new QWidget(scroll);
    m_listContent->setObjectName("listContent");
    m_itemsLayout = new QVBoxLayout(m_listContent);
    m_itemsLayout->setContentsMargins(0, 0, 0, 0);
    m_itemsLayout->setSpacing(0);
    m_itemsLayout->addStretch(1);
    scroll->setWidget(m_listContent);
    root->addWidget(scroll, 1);

    m_emptyLabel = new QLabel(QStringLiteral("No conversations found"), this);
    m_emptyLabel->setObjectName("listEmpty");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setWordWrap(true);
    m_emptyLabel->setVisible(false);
    root->addWidget(m_emptyLabel, 1);

    if (m_themeManager) {
        connect(m_themeManager, &ThemeManager::appearanceChanged, this,
                &ConversationList::refreshTheme);
    }
    refreshTheme();
}

void ConversationList::setRepository(IChatRepository* repository) {
    if (m_repository) {
        disconnect(m_repository, nullptr, this, nullptr);
    }
    m_repository = repository;
    if (m_repository) {
        connect(m_repository, &IChatRepository::conversationsChanged, this,
                &ConversationList::rebuildList);
        connect(m_repository, &IChatRepository::conversationAdded, this,
                &ConversationList::onConversationAdded);
        rebuildList();
    }
}

QString ConversationList::selectedConversationId() const {
    return m_selectedId;
}

void ConversationList::setSelectedConversation(const QString& conversationId) {
    if (m_selectedId == conversationId) {
        return;
    }
    if (m_items.contains(m_selectedId)) {
        m_items.value(m_selectedId)->setSelected(false);
    }
    m_selectedId = conversationId;
    if (m_items.contains(m_selectedId)) {
        m_items.value(m_selectedId)->setSelected(true);
    }
}

void ConversationList::applyFilter(const QString& text) {
    m_filter = text.trimmed();
    if (!m_repository) {
        return;
    }
    for (const auto& conversation : m_repository->conversations()) {
        ConversationItemWidget* widget = m_items.value(conversation.id, nullptr);
        if (widget) {
            widget->setVisible(matchesFilter(conversation));
        }
    }
    updateEmptyState();
}

void ConversationList::openCreateGroup() {
    if (!m_repository) {
        return;
    }
    CreateGroupDialog dialog(m_themeManager, m_repository->contacts(), this);
    if (dialog.exec() != QDialog::Accepted) {
        return;
    }
    m_repository->createGroup(dialog.groupName(), dialog.selectedContactIds());
}

void ConversationList::rebuildList() {
    if (!m_repository) {
        return;
    }
    const QVector<chat::Conversation> all = m_repository->conversations();

    const QList<QString> knownIds = m_items.keys();
    for (const QString& id : knownIds) {
        bool stillExists = false;
        for (const auto& conversation : all) {
            if (conversation.id == id) {
                stillExists = true;
                break;
            }
        }
        if (!stillExists) {
            ConversationItemWidget* widget = m_items.take(id);
            m_itemsLayout->removeWidget(widget);
            widget->deleteLater();
            if (m_selectedId == id) {
                m_selectedId.clear();
            }
        }
    }

    int position = 0;
    for (const auto& conversation : all) {
        ConversationItemWidget* widget = m_items.value(conversation.id, nullptr);
        if (!widget) {
            widget = new ConversationItemWidget(m_themeManager, conversation, m_listContent);
            connect(widget, &ConversationItemWidget::pressed, this,
                    [this](const QString& id) {
                        setSelectedConversation(id);
                        if (m_repository) {
                            m_repository->markAsRead(id);
                        }
                        emit conversationSelected(id);
                    });
            m_items.insert(conversation.id, widget);
        } else {
            widget->updateConversation(conversation);
        }
        m_itemsLayout->removeWidget(widget);
        m_itemsLayout->insertWidget(position, widget);
        widget->setSelected(conversation.id == m_selectedId);
        widget->setVisible(matchesFilter(conversation));
        ++position;
    }
    updateEmptyState();
}

void ConversationList::onConversationAdded(const QString& conversationId) {
    m_filter.clear();
    rebuildList();
    setSelectedConversation(conversationId);
    if (m_repository) {
        m_repository->markAsRead(conversationId);
    }
    emit conversationSelected(conversationId);
}

void ConversationList::refreshTheme() {
    if (!m_themeManager) {
        return;
    }
    const ThemeManager::SurfaceColors surfaces = m_themeManager->surfaceColors();
    const QColor topBar = m_themeManager->currentPalette().topBarBackground;
    setStyleSheet(QStringLiteral(
        "ConversationList#conversationList { background-color: %1; border: none; }"
        "QLabel#listEmpty { background: transparent; border: none; color: %2; }"
        "QScrollArea#listScroll { background: transparent; border: none; }"
        "QWidget#listContent { background: transparent; border: none; }"
        "QScrollBar:vertical { background: transparent; width: 8px; margin: 2px; }"
        "QScrollBar::handle:vertical { background: %3; border-radius: 4px; min-height: 24px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }")
        .arg(topBar.name(), surfaces.muted.name(), surfaces.stroke.name()));
}

bool ConversationList::matchesFilter(const chat::Conversation& conversation) const {
    if (m_filter.isEmpty()) {
        return true;
    }
    return conversation.name.contains(m_filter, Qt::CaseInsensitive) ||
        conversation.preview.contains(m_filter, Qt::CaseInsensitive);
}

void ConversationList::updateEmptyState() {
    bool anyVisible = false;
    for (auto it = m_items.constBegin(); it != m_items.constEnd(); ++it) {
        if (it.value() && it.value()->isVisible()) {
            anyVisible = true;
            break;
        }
    }
    const bool empty = m_items.isEmpty() || !anyVisible;
    m_emptyLabel->setVisible(empty);
}
