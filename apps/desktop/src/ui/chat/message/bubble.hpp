#pragma once

#include <QWidget>

#include "chat/chat_models.hpp"

class QLabel;
class ThemeManager;

class MessageBubble final : public QWidget {
    Q_OBJECT

public:
    explicit MessageBubble(
        ThemeManager* themeManager,
        const chat::Message& message,
        QWidget* parent = nullptr);

    void setMaxBubbleWidth(int width);
    void refreshTheme();

private:
    ThemeManager* m_themeManager = nullptr;
    chat::Message m_message;
    QLabel* m_textLabel = nullptr;
    QLabel* m_timeLabel = nullptr;
    QWidget* m_bubble = nullptr;
};
