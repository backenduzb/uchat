#pragma once

#include <QWidget>

class AnimationManager;
class Button;
class IconButton;
class QLabel;
class ThemeManager;

class MenuDrawer final : public QWidget {
    Q_OBJECT

public:
    explicit MenuDrawer(ThemeManager* themeManager, AnimationManager* animationManager,
                        QWidget* parent = nullptr);

    void setUserInfo(const QString& displayName, const QString& username);
    [[nodiscard]] bool isOpen() const noexcept;

public slots:
    void open();
    void close();
    void toggle();

signals:
    void logoutRequested();
    void editThemeRequested();
    void drawerOpened();
    void drawerClosed();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void fitToParent();
    void refreshUserLabels();
    void refreshThemeIcon();

    static constexpr int kPanelWidth = 300;

    ThemeManager* m_themeManager = nullptr;
    AnimationManager* m_animationManager = nullptr;

    QWidget* m_dim = nullptr;
    QWidget* m_panel = nullptr;

    IconButton* m_logoutButton = nullptr;
    IconButton* m_closeButton = nullptr;
    QLabel* m_avatarLabel = nullptr;
    QLabel* m_nameLabel = nullptr;
    QLabel* m_usernameLabel = nullptr;

    Button* m_editThemeButton = nullptr;

    QString m_displayName;
    QString m_username;

    bool m_open = false;
    bool m_animating = false;
};
