#pragma once

#include <QObject>

enum class AppStatus {
    Unauthenticated,
    Authenticated
};

class AppState final : public QObject
{
    Q_OBJECT

public:
    explicit AppState(QObject* parent = nullptr);

    [[nodiscard]] AppStatus status() const noexcept;

public slots:
    void setStatus(AppStatus status);

signals:
    void statusChanged(AppStatus status);

private:
    AppStatus m_status = AppStatus::Unauthenticated;
};
