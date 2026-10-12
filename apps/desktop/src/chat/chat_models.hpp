#pragma once

#include <QColor>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QVector>

#include <cmath>

namespace chat {

inline constexpr const char* kLocalUserId = "me";

enum class ConversationType {
    Private,
    Group,
};

struct Contact {
    QString id;
    QString displayName;
};

struct Message {
    QString id;
    QString conversationId;
    QString senderId;
    QString text;
    QDateTime timestamp;

    [[nodiscard]] bool isOutgoing() const noexcept {
        return senderId == QLatin1String(kLocalUserId);
    }
};

struct Conversation {
    QString id;
    QString name;
    ConversationType type = ConversationType::Private;
    QString preview;
    QDateTime lastActivity;
    int unreadCount = 0;
    QStringList participantIds;

    [[nodiscard]] bool isGroup() const noexcept {
        return type == ConversationType::Group;
    }
};

[[nodiscard]] inline QString contactInitials(const QString& name) {
    const QStringList parts =
        name.split(QLatin1Char(' '), Qt::SkipEmptyParts);
    QString initials;
    for (const QString& part : parts) {
        if (initials.size() >= 2) {
            break;
        }
        if (!part.isEmpty()) {
            initials += part.left(1).toUpper();
        }
    }
    return initials.isEmpty() ? QStringLiteral("?") : initials;
}

[[nodiscard]] inline QColor avatarColorFor(const QString& id) {
    static const QColor kPalette[] = {
        QColor("#C65D5D"), QColor("#C98A3B"), QColor("#7C6CF0"), QColor("#4E9B6A"),
        QColor("#3E8FA6"), QColor("#5B8DEF"), QColor("#B45F8E"),
    };
    if (id.isEmpty()) {
        return kPalette[0];
    }
    quint32 hash = 0;
    const QString lowered = id.toLower();
    for (const QChar c : lowered) {
        hash = hash * 31 + static_cast<quint32>(c.unicode());
    }
    constexpr int paletteSize = static_cast<int>(sizeof(kPalette) / sizeof(kPalette[0]));
    return kPalette[static_cast<int>(hash % static_cast<quint32>(paletteSize))];
}

[[nodiscard]] inline QColor avatarTextColorFor(const QColor& background) {
    if (!background.isValid()) {
        return QColor("#FFFFFF");
    }
    const auto linear = [](int channel) {
        const double c = static_cast<double>(channel) / 255.0;
        return c <= 0.03928 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
    };
    const double luminance = 0.2126 * linear(background.red()) +
        0.7152 * linear(background.green()) + 0.0722 * linear(background.blue());
    return luminance > 0.35 ? QColor("#1A1A1A") : QColor("#FFFFFF");
}

[[nodiscard]] inline QString formatListTime(const QDateTime& time) {
    if (!time.isValid()) {
        return {};
    }
    const QDate today = QDate::currentDate();
    const QDate date = time.date();
    if (date == today) {
        return time.toString(QStringLiteral("HH:mm"));
    }
    if (date == today.addDays(-1)) {
        return QStringLiteral("Yesterday");
    }
    if (date.daysTo(today) < 7) {
        return time.toString(QStringLiteral("dddd"));
    }
    return time.toString(QStringLiteral("dd/MM/yy"));
}

[[nodiscard]] inline QString formatMessageTime(const QDateTime& time) {
    if (!time.isValid()) {
        return {};
    }
    return time.toString(QStringLiteral("HH:mm"));
}

} // namespace chat
