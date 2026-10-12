#pragma once

#include <QObject>
#include <QString>
#include <qobject.h>

class QNetworkAccessManager;
class QNetworkReply;

class Authentication final : public QObject {
    Q_OBJECT

  public:
    explicit Authentication(QObject* parent = nullptr);
    void saveSession(const QString& token, const QString& username, const QString& userId);
    void clearSession();
    bool checkExistingSession();
    QString getStoredToken() const;
    void login(const QString& email, const QString& password);

  signals:
    void loginSucceeded(const QString& accessToken, const QString& username, const QString& userId);

    void loginFailed(const QString& message);

    void loginStarted();

  private:
    QNetworkAccessManager* m_networkManager = nullptr;

  private slots:
    void handleLoginResponse(QNetworkReply* reply);
};
