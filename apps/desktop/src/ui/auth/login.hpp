#pragma once

#include <QWidget>

class QLineEdit;
class QLabel;
class QPushButton;

class LoginWindow final : public QWidget {
    Q_OBJECT
    public:
    explicit LoginWindow(QWidget *parent = nullptr);
    signals:
    void loginRequested(const QString& username, const QString& password);
    private slots:
        void submitLogin();
    private:
        QLineEdit *m_usernameInput = nullptr;
        QLineEdit *m_passwordInput = nullptr;
        QPushButton *m_loginButton = nullptr;
        QLabel *m_errorLabel = nullptr;
};
