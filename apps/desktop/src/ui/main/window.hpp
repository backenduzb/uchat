#pragma once

#include <QMainWindow>
#include <QPointer>

class QSplitter;
class QStackedWidget;
class QVariantAnimation;
class QWidget;
class ThemeManager;
class AnimationManager;
class LoginWindow;
class Authentication;
class ChatArea;
class ConversationList;
class MockChatRepository;
class TopBar;
class MenuDrawer;
class ThemeEditorOverlay;

class MainWindow final : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(
        ThemeManager* themeManager,
        AnimationManager* animationManager,
        QWidget* parent = nullptr
    );

private:
    void showChatPage();
    void showLoginPage();
    void handleLogout();
    void onConversationSelected(const QString& conversationId);
    void onBackToList();
    void updateNarrowMode();
    void slideNarrowTo(bool showChat);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

    QStackedWidget* m_pages = nullptr;
    LoginWindow* m_loginPage = nullptr;
    QWidget* m_chatPage = nullptr;
    TopBar* m_topBar = nullptr;
    MenuDrawer* m_menuDrawer = nullptr;
    ThemeEditorOverlay* m_themeOverlay = nullptr;
    QSplitter* m_splitter = nullptr;
    QWidget* m_leftColumn = nullptr;
    ConversationList* m_conversationList = nullptr;
    ChatArea* m_chatArea = nullptr;

    bool m_narrowMode = false;
    bool m_narrowChatShown = false;
    QPointer<QVariantAnimation> m_slideAnimation;

    MockChatRepository* m_chatRepository = nullptr;

    ThemeManager* m_themeManager = nullptr;
    AnimationManager* m_animationManager = nullptr;
    Authentication *m_authentication = nullptr;    
};