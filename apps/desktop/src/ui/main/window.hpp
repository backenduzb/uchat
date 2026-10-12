#pragma once

#include <QMainWindow>

class QStackedWidget;
class QWidget;
class ThemeManager;
class AnimationManager;
class LoginWindow;
class Authentication;

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

    QStackedWidget* m_pages = nullptr;
    LoginWindow* m_loginPage = nullptr;
    QWidget* m_chatPage = nullptr;

    ThemeManager* m_themeManager = nullptr;
    AnimationManager* m_animationManager = nullptr;
    Authentication *m_authentication = nullptr;
};