#pragma once

#include "animation/AnimationManager.hpp"
#include <QFrame>
#include <QWidget>

class ThemeManager;
class InputField;
class Button;
class QLabel;
class QPushButton;
class AnimationManager;

class LoginWindow final : public QWidget {
    Q_OBJECT

  public:
    explicit LoginWindow(ThemeManager* themeManager, AnimationManager* animationManager, QWidget* parent = nullptr);
    void showLoginError(const QString& message);

  signals:
    void loginRequested(const QString& username, const QString& password);

  private slots:
    void submitLogin();
    void goToPasswordStep();
    void goToUsernameStep();

  protected:
    void keyPressEvent(QKeyEvent* event) override;

  private:
    enum class Step {
        Username,
        Password,
    };

    void applyStepContent(Step step);

    ThemeManager* m_themeManager = nullptr;

    AnimationManager* m_animationManager = nullptr;
    QFrame* m_loginCard = nullptr;
    QWidget* m_fieldSlot = nullptr;
    QLabel* m_subtitleLabel = nullptr;
    InputField* m_usernameInput = nullptr;
    InputField* m_passwordInput = nullptr;
    Button* m_loginButton = nullptr;

    QPushButton* m_themeDarkButton = nullptr;
    QPushButton* m_themeLightButton = nullptr;
    QPushButton* m_themeMonochromeButton = nullptr;

    QLabel* m_errorLabel = nullptr;

    Step m_step = Step::Username;
    bool m_swapping = false;
    void showError(const QString& message);
};
