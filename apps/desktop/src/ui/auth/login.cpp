#include "login.hpp"
#include "animation/AnimationManager.hpp"
#include "components/Button.hpp"
#include "components/InputField.hpp"
#include "theme/ThemeManager.hpp"
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QVBoxLayout>

LoginWindow::LoginWindow(ThemeManager* themeManager, AnimationManager* animationManager,
                         QWidget* parent)
    : QWidget(parent), m_themeManager(themeManager), m_animationManager(animationManager) {
    setWindowTitle("Sign in");
    setMinimumSize(600, 560);
    resize(460, 600);

    setObjectName("loginWindow");

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 28);

    auto* card = new QFrame(this);
    card->setObjectName("loginCard");
    card->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    card->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(28, 30, 28, 30);
    layout->setSpacing(14);

    auto* logo = new QLabel("Chat");
    logo->setObjectName("loginLogo");

    QFont logoFont = logo->font();
    logoFont.setPointSize(15);
    logoFont.setBold(true);
    logo->setFont(logoFont);

    auto* title = new QLabel("Welcome back");
    title->setObjectName("loginTitle");

    QFont titleFont = title->font();
    titleFont.setPointSize(23);
    titleFont.setBold(true);
    title->setFont(titleFont);

    m_subtitleLabel = new QLabel("Enter username");
    m_subtitleLabel->setObjectName("loginSubtitle");
    m_subtitleLabel->setWordWrap(true);

    m_fieldSlot = new QWidget(card);

    auto* fieldLayout = new QVBoxLayout(m_fieldSlot);
    fieldLayout->setContentsMargins(0, 0, 0, 0);
    fieldLayout->setSpacing(0);

    m_usernameInput = new InputField(m_fieldSlot, m_themeManager);
    m_usernameInput->setObjectName("usernameInput");
    m_usernameInput->setPlaceholderText("Email or username");
    m_usernameInput->setClearButtonEnabled(true);

    m_passwordInput = new InputField(m_fieldSlot, m_themeManager);
    m_passwordInput->setObjectName("passwordInput");
    m_passwordInput->setPlaceholderText("Password");
    m_passwordInput->setEchoMode(QLineEdit::Password);
    m_passwordInput->hide();

    fieldLayout->addWidget(m_usernameInput);
    fieldLayout->addWidget(m_passwordInput);

    m_errorLabel = new QLabel;
    m_errorLabel->setObjectName("errorLabel");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->hide();

    m_loginButton = new Button("Continue", card, m_themeManager);
    m_loginButton->setObjectName("loginButton");
    m_loginButton->setCursor(Qt::PointingHandCursor);

    layout->addWidget(logo);
    layout->addSpacing(10);
    layout->addWidget(title);
    layout->addWidget(m_subtitleLabel);
    layout->addSpacing(12);
    layout->addWidget(m_fieldSlot);
    layout->addWidget(m_errorLabel);
    layout->addSpacing(6);
    layout->addWidget(m_loginButton);

    root->addStretch();
    root->addWidget(card, 0, Qt::AlignCenter);
    root->addStretch();

    auto* themeLabel = new QLabel("Appearance");
    themeLabel->setObjectName("themeLabel");

    m_themeDarkButton = new QPushButton;
    m_themeDarkButton->setObjectName("themeDark");
    m_themeDarkButton->setFixedSize(28, 28);
    m_themeDarkButton->setCursor(Qt::PointingHandCursor);
    m_themeDarkButton->setToolTip("Dark theme");
    m_themeDarkButton->setAccessibleName("Dark theme");

    m_themeLightButton = new QPushButton;
    m_themeLightButton->setObjectName("themeLight");
    m_themeLightButton->setFixedSize(28, 28);
    m_themeLightButton->setCursor(Qt::PointingHandCursor);
    m_themeLightButton->setToolTip("Light theme");
    m_themeLightButton->setAccessibleName("Light theme");

    auto* bottomLayout = new QHBoxLayout;
    bottomLayout->setSpacing(8);
    bottomLayout->addStretch();
    bottomLayout->addWidget(themeLabel);
    bottomLayout->addWidget(m_themeDarkButton);
    bottomLayout->addWidget(m_themeLightButton);

    root->addLayout(bottomLayout);

    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::submitLogin);
    connect(m_usernameInput, &QLineEdit::returnPressed, this, &LoginWindow::submitLogin);
    connect(m_passwordInput, &QLineEdit::returnPressed, this, &LoginWindow::submitLogin);

    if (m_themeManager) {
        connect(m_themeDarkButton, &QPushButton::clicked, this,
                [this]() { m_themeManager->setTheme(ThemeManager::Theme::Dark); });

        connect(m_themeLightButton, &QPushButton::clicked, this,
                [this]() { m_themeManager->setTheme(ThemeManager::Theme::Light); });
    }

    m_usernameInput->setFocus();
}

#include <QRegularExpression>

void LoginWindow::submitLogin() {
    if (m_swapping) {
        return;
    }

    if (m_step == Step::Username) {
        const QString email = m_usernameInput->text().trimmed();

        if (email.isEmpty()) {
            showError("Please enter your email.");
            return;
        }

        static const QRegularExpression emailRegex(
            R"(^[A-Za-z0-9.!#$%&'*+/=?^_`{|}~-]+@[A-Za-z0-9-]+(\.[A-Za-z0-9-]+)+$)");

        if (!emailRegex.match(email).hasMatch()) {
            showError("Please enter a valid email address.");
            return;
        }

        m_errorLabel->hide();
        goToPasswordStep();
        return;
    }

    const QString email = m_usernameInput->text().trimmed();
    const QString password = m_passwordInput->text();

    if (password.isEmpty()) {
        showError("Please enter your password.");
        return;
    }

    m_errorLabel->hide();
    m_loginButton->setLoading(true);
    emit loginRequested(email, password);
}
void LoginWindow::goToPasswordStep() {
    if (m_swapping || m_step == Step::Password) {
        return;
    }
    m_swapping = true;
    m_animationManager->slideReplace(
        m_fieldSlot, [this]() { applyStepContent(Step::Password); },
        AnimationManager::SlideDirection::Forward, 48, 320,
        [this]() {
            m_swapping = false;
            m_passwordInput->setFocus();
        });
}

void LoginWindow::goToUsernameStep() {
    if (m_swapping || m_step == Step::Username) {
        return;
    }
    m_swapping = true;
    m_animationManager->slideReplace(
        m_fieldSlot, [this]() { applyStepContent(Step::Username); },
        AnimationManager::SlideDirection::Backward, 48, 320,
        [this]() {
            m_swapping = false;
            m_usernameInput->setFocus();
        });
}

void LoginWindow::applyStepContent(Step step) {
    m_step = step;
    m_errorLabel->hide();

    const bool isPassword = (step == Step::Password);
    m_usernameInput->setVisible(!isPassword);
    m_passwordInput->setVisible(isPassword);
    m_subtitleLabel->setText(isPassword ? "Enter password" : "Enter username");
    m_loginButton->setText(isPassword ? "Sign in" : "Continue");
}

void LoginWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape && m_step == Step::Password && !m_swapping) {
        goToUsernameStep();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void LoginWindow::showLoginError(const QString& message) {
    m_errorLabel->setText(message);
    m_errorLabel->show();
    m_loginButton->setLoading(false);

    m_animationManager->slideUp(m_errorLabel);
    m_animationManager->shake(m_loginButton);
}

void LoginWindow::showError(const QString& message) {
    m_loginButton->setLoading(false);
    m_errorLabel->setText(message);
    m_errorLabel->show();
    m_animationManager->slideUp(m_errorLabel);
    m_animationManager->shake(m_loginButton);
}