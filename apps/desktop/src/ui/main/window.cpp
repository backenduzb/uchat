#include "window.hpp"

#include "animation/AnimationManager.hpp"
#include "chat/mock_chat_repository.hpp"
#include "http/auth/Authorization.hpp"
#include "theme/ThemeManager.hpp"
#include "ui/auth/login.hpp"
#include "ui/chat/area/chat_area.hpp"
#include "ui/chat/conversation/list.hpp"
#include "ui/components/layout/MenuDrawer.hpp"
#include "ui/components/layout/TopBar.hpp"
#include "ui/theme/ThemeEditor.hpp"

#include <QHBoxLayout>
#include <QEvent>
#include <QSplitter>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <qsettings.h>

namespace {

constexpr int kDefaultListWidth = 320;
constexpr int kMinListWidth = 240;
constexpr int kMaxListWidth = 420;
constexpr int kNarrowThreshold = 680;
constexpr int kSlideDurationMs = 240;

}

MainWindow::MainWindow(ThemeManager* themeManager, AnimationManager* animationManager,
                       QWidget* parent)
    : QMainWindow(parent), m_themeManager(themeManager), m_animationManager(animationManager) {
    setWindowTitle("Uchat");
    resize(1100, 720);
    setMinimumSize(480, 520);

    m_authentication = new Authentication(this);
    m_pages = new QStackedWidget(this);
    setCentralWidget(m_pages);

    m_loginPage = new LoginWindow(m_themeManager, m_animationManager, m_pages);

    m_chatPage = new QWidget(m_pages);
    auto* chatLayout = new QVBoxLayout(m_chatPage);
    chatLayout->setContentsMargins(0, 0, 0, 0);
    chatLayout->setSpacing(0);

    m_topBar = new TopBar(m_themeManager, m_animationManager, m_chatPage);

    m_chatRepository = new MockChatRepository(this);

    m_splitter = new QSplitter(Qt::Horizontal, m_chatPage);
    m_splitter->setObjectName("chatSplitter");
    m_splitter->setHandleWidth(1);
    m_splitter->setChildrenCollapsible(false);

    m_leftColumn = new QWidget(m_splitter);
    m_leftColumn->setObjectName("leftColumn");
    auto* leftLayout = new QVBoxLayout(m_leftColumn);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(0);
    m_topBar->setParent(m_leftColumn);
    leftLayout->addWidget(m_topBar);

    m_conversationList = new ConversationList(m_themeManager, m_leftColumn);
    m_conversationList->setRepository(m_chatRepository);
    leftLayout->addWidget(m_conversationList, 1);

    m_chatArea = new ChatArea(m_themeManager, m_splitter);
    m_chatArea->setRepository(m_chatRepository);

    m_splitter->addWidget(m_leftColumn);
    m_splitter->addWidget(m_chatArea);
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    m_splitter->setSizes({kDefaultListWidth, 720});
    m_leftColumn->setMinimumWidth(kMinListWidth);
    m_leftColumn->setMaximumWidth(kMaxListWidth);

    if (m_themeManager) {
        connect(m_themeManager, &ThemeManager::appearanceChanged, this, [this]() {
            if (m_splitter && m_themeManager) {
                const QColor stroke = m_themeManager->surfaceColors().stroke;
                m_splitter->setStyleSheet(QStringLiteral(
                    "QSplitter#chatSplitter { background-color: %1; }").arg(stroke.name()));
            }
        });
        const QColor stroke = m_themeManager->surfaceColors().stroke;
        m_splitter->setStyleSheet(QStringLiteral(
            "QSplitter#chatSplitter { background-color: %1; }").arg(stroke.name()));
    }
    chatLayout->addWidget(m_splitter, 1);

    connect(m_conversationList, &ConversationList::conversationSelected, this,
            &MainWindow::onConversationSelected);
    connect(m_chatArea, &ChatArea::backRequested, this, &MainWindow::onBackToList);
    connect(m_topBar, &TopBar::searchTextChanged, m_conversationList,
            &ConversationList::applyFilter);
    connect(m_topBar, &TopBar::createGroupRequested, m_conversationList,
            &ConversationList::openCreateGroup);

    m_chatPage->installEventFilter(this);

    m_menuDrawer = new MenuDrawer(m_themeManager, m_animationManager, m_chatPage);

    m_themeOverlay = new ThemeEditorOverlay(m_themeManager, m_animationManager, m_chatPage);

    connect(m_topBar, &TopBar::menuClicked, m_menuDrawer, &MenuDrawer::open);
    connect(m_menuDrawer, &MenuDrawer::logoutRequested, this, &MainWindow::handleLogout);
    connect(m_menuDrawer, &MenuDrawer::editThemeRequested, m_themeOverlay,
            &ThemeEditorOverlay::openEditor);

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
            m_menuDrawer->setUserInfo(savedUsername, savedUsername);
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
                m_menuDrawer->setUserInfo(username, username);
                showChatPage();
            });

    updateNarrowMode();
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

void MainWindow::handleLogout() {
    if (m_menuDrawer && m_menuDrawer->isOpen()) {
        m_menuDrawer->close();
    }
    if (m_authentication) {
        m_authentication->clearSession();
    }
    setWindowTitle(QStringLiteral("Uchat"));
    showLoginPage();
}

void MainWindow::onConversationSelected(const QString& conversationId) {
    if (m_chatArea) {
        m_chatArea->setConversation(conversationId);
    }
    if (m_narrowMode && !m_narrowChatShown) {
        slideNarrowTo(true);
    }
}

void MainWindow::onBackToList() {
    if (!m_narrowMode || !m_narrowChatShown) {
        return;
    }
    slideNarrowTo(false);
}

void MainWindow::updateNarrowMode() {
    if (!m_chatPage || !m_leftColumn || !m_chatArea || !m_splitter) {
        return;
    }
    const bool narrow = m_chatPage->width() < kNarrowThreshold;
    m_chatArea->setShowBackButton(narrow);
    if (narrow) {
        m_leftColumn->setMinimumWidth(0);
        m_leftColumn->setMaximumWidth(QWIDGETSIZE_MAX);
    } else {
        m_leftColumn->setMinimumWidth(kMinListWidth);
        m_leftColumn->setMaximumWidth(kMaxListWidth);
    }
    if (narrow == m_narrowMode) {
        return;
    }
    m_narrowMode = narrow;
    AnimationManager::stopAnimation(m_slideAnimation);
    if (m_narrowMode) {
        m_narrowChatShown = false;
        m_chatArea->setVisible(false);
        m_leftColumn->setVisible(true);
    } else {
        m_leftColumn->setVisible(true);
        m_chatArea->setVisible(true);
    }
}

void MainWindow::slideNarrowTo(bool showChat) {
    if (!m_splitter || !m_leftColumn || !m_chatArea) {
        return;
    }
    AnimationManager::stopAnimation(m_slideAnimation);
    m_leftColumn->setVisible(true);
    m_chatArea->setVisible(true);
    const QList<int> current = m_splitter->sizes();
    int total = 0;
    for (const int size : current) {
        total += size;
    }
    if (total <= 0) {
        total = m_splitter->width();
    }
    const int fromLeft = current.value(0, total);
    const int targetLeft = showChat ? 0 : total;
    m_narrowChatShown = showChat;
    if (fromLeft == targetLeft || total <= 0) {
        m_leftColumn->setVisible(!showChat);
        m_chatArea->setVisible(showChat);
        return;
    }
    m_slideAnimation = AnimationManager::animateProgress(
        0.0, 1.0, kSlideDurationMs, this,
        [this, fromLeft, targetLeft, total, showChat](double progress) {
            if (!m_splitter || !m_leftColumn || !m_chatArea) {
                return;
            }
            const int left = fromLeft + static_cast<int>(
                static_cast<double>(targetLeft - fromLeft) * progress);
            m_splitter->setSizes({left, total - left});
            if (progress >= 1.0) {
                m_leftColumn->setVisible(!showChat);
                m_chatArea->setVisible(showChat);
            }
        },
        QEasingCurve(QEasingCurve::OutCubic));
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_chatPage && event && event->type() == QEvent::Resize) {
        updateNarrowMode();
    }
    return QMainWindow::eventFilter(watched, event);
}
