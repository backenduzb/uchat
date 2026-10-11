#include "AppState.hpp"

AppState::AppState(QObject* parent)
    : QObject(parent)
{
}

AppStatus AppState::status() const noexcept
{
    return m_status;
}

void AppState::setStatus(AppStatus status)
{
    if (m_status == status)
        return;

    m_status = status;
    emit statusChanged(m_status);
}
