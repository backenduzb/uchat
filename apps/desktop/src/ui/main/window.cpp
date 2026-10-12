#include "window.hpp"

#include "animation/AnimationManager.hpp"
#include "http/auth/Authorization.hpp"
#include "theme/ThemeManager.hpp"
#include "ui/auth/login.hpp"

#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <qsettings.h>

MainWindow::MainWindow(ThemeManager* themeManager, AnimationManager* animationManager,
                       QWidget* parent)
    : QMainWindow(parent), m_themeManager(themeManager), m_animationManager(animationManager) {
    setWindowTitle("Uchat");
    resize(1100, 720);
    setMinimumSize(800, 500);

    m_authentication = new Authentication(this);
    m_pages = new QStackedWidget(this);
    setCentralWidget(m_pages);

    m_loginPage = new LoginWindow(m_themeManager, m_animationManager, m_pages);

    m_chatPage = new QWidget(m_pages);
    auto* chatLayout = new QVBoxLayout(m_chatPage);
    auto* welcomeLabel = new QLabel("Welcome to Uchat", m_chatPage);

    welcomeLabel->setAlignment(Qt::AlignCenter);
    chatLayout->addWidget(welcomeLabel);

    m_pages->addWidget(m_loginPage);
    m_pages->addWidget(m_chatPage);

    if (m_themeManager) {
        m_themeManager->registerPage(m_loginPage, ":/styles/auth/login.qss");
    }

    if (m_authentication->checkExistingSession()) {
        m_pages->setCurrentWidget(m_chatPage);

        QSettings settings("Uchat", "UchatApp");
        QString savedUsername = settings.value("auth/username").toString();
        if (!savedUsername.isEmpty()) {
            setWindowTitle(QStringLiteral("Uchat | %1").arg(savedUsername));
        }
    } else {
        m_pages->setCurrentWidget(m_loginPage);
    }

    connect(m_loginPage, &LoginWindow::loginRequested, m_authentication, &Authentication::login);

    connect(m_authentication, &Authentication::loginFailed, m_loginPage,
            &LoginWindow::showLoginError);

    connect(m_authentication, &Authentication::loginSucceeded, this,
            [this](const QString& accessToken, const QString& username, const QString& userId) {
                m_authentication->saveSession(accessToken, username, userId);
                setWindowTitle(QStringLiteral("Uchat | %1").arg(username));
                showChatPage();
    });
}

void MainWindow::showChatPage() {
    if (!m_pages || !m_chatPage) {
        return;
    }

    if (m_pages->currentWidget() == m_chatPage) {
        return;
    }

    if (m_animationManager) {
        m_animationManager->slideReplace(
            m_pages, [this]() { m_pages->setCurrentWidget(m_chatPage); },
            AnimationManager::SlideDirection::Forward, 80, 400);
    } else {
        m_pages->setCurrentWidget(m_chatPage);
    }
}

void MainWindow::showLoginPage() {
    if (!m_pages || !m_loginPage) {
        return;
    }

    if (m_animationManager) {
        m_animationManager->slideReplace(
            m_pages, [this]() { m_pages->setCurrentWidget(m_loginPage); },
            AnimationManager::SlideDirection::Backward, 80, 400);
    } else {
        m_pages->setCurrentWidget(m_loginPage);
    }
}
