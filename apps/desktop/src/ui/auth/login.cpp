#include "login.hpp"

#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

LoginWindow::LoginWindow(QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle("Sign in");
    setMinimumSize(420, 560);
    resize(460, 600);

    
    setStyleSheet(R"(
        QWidget {
            background-color: #050505;
            color: #FFFFFF;
            font-size: 10pt;
        }
    
        QFrame#loginCard {
            background-color: rgba(255, 255, 255, 0.05);
            border: 1px solid rgba(255, 255, 255, 0.15);
            border-radius: 20px;
        }
    
        QLineEdit {
            background-color: rgba(255, 255, 255, 0.07);
            color: #FFFFFF;
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 10px;
            padding: 12px 16px;
            selection-background-color: rgba(255, 255, 255, 0.3);
            selection-color: #FFFFFF;
        }
    
        QLineEdit:focus {
            border: 1px solid rgba(255, 255, 255, 0.6);
            background-color: rgba(255, 255, 255, 0.12);
        }
    
        QLineEdit::placeholder {
            color: rgba(255, 255, 255, 0.4);
        }
    
        QPushButton {
            background-color: #FFFFFF;
            color: #000000;
            border: 1px solid #FFFFFF;
            border-radius: 10px;
            padding: 12px;
            font-weight: 700;
        }
    
        QPushButton:hover {
            background-color: rgba(255, 255, 255, 0.85);
            color: #000000;
            border: 1px solid #FFFFFF;
        }
    
        QPushButton:pressed {
            background-color: rgba(255, 255, 255, 0.6);
            color: #000000;
        }
    
        QPushButton:disabled {
            background-color: rgba(255, 255, 255, 0.05);
            color: rgba(255, 255, 255, 0.3);
            border: 1px solid rgba(255, 255, 255, 0.1);
        }
    )");


    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(28, 28, 28, 28);

    auto* card = new QFrame(this);
    card->setObjectName("loginCard");

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(28, 30, 28, 30);
    layout->setSpacing(14);

    auto* logo = new QLabel("Chat");
    QFont logoFont = logo->font();
    logoFont.setPointSize(15);
    logoFont.setBold(true);
    logo->setFont(logoFont);

    auto* title = new QLabel("Welcome back");
    QFont titleFont = title->font();
    titleFont.setPointSize(23);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto* subtitle = new QLabel(
        "Sign in to continue to your conversations."
    );
    subtitle->setWordWrap(true);
    subtitle->setStyleSheet("color: #FFFFFF;  background: transparent;");
    

    m_usernameInput = new QLineEdit;
    m_usernameInput->setPlaceholderText("Email or username");
    m_usernameInput->setClearButtonEnabled(true);

    m_passwordInput = new QLineEdit;
    m_passwordInput->setPlaceholderText("Password");
    m_passwordInput->setEchoMode(QLineEdit::Password);

    m_errorLabel = new QLabel;
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setStyleSheet("color: #F87171; background: transparent;");
    m_errorLabel->hide();

    m_loginButton = new QPushButton("Sign in");
    m_loginButton->setCursor(Qt::PointingHandCursor);

    layout->addWidget(logo);
    layout->addSpacing(10);
    layout->addWidget(title);
    layout->addWidget(subtitle);
    layout->addSpacing(12);
    layout->addWidget(m_usernameInput);
    layout->addWidget(m_passwordInput);
    layout->addWidget(m_errorLabel);
    layout->addSpacing(6);
    layout->addWidget(m_loginButton);

    root->addStretch();
    root->addWidget(card);
    root->addStretch();

    connect(
        m_loginButton,
        &QPushButton::clicked,
        this,
        &LoginWindow::submitLogin
    );

    connect(
        m_passwordInput,
        &QLineEdit::returnPressed,
        this,
        &LoginWindow::submitLogin
    );
}

void LoginWindow::submitLogin()
{
    const QString username = m_usernameInput->text().trimmed();
    const QString password = m_passwordInput->text();

    if (username.isEmpty() || password.isEmpty()) {
        m_errorLabel->setText(
            "Please enter your username and password."
        );
        m_errorLabel->show();
        return;
    }

    m_errorLabel->hide();

    emit loginRequested(username, password);
}
