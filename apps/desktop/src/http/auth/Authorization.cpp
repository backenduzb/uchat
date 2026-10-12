#include "Authorization.hpp"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QUrl>

namespace {
const QUrl kLoginUrl(QStringLiteral("https://uchat-identication.up.railway.app/auth/login"));
const QString kTokenKey = QStringLiteral("auth/access_token");
const QString kUsernameKey = QStringLiteral("auth/username");
const QString kUserIdKey = QStringLiteral("auth/user_id");
} // namespace

void Authentication::saveSession(const QString& token, const QString& username,
                                 const QString& userId) {
    QSettings settings("Uchat", "UchatApp");
    settings.setValue(kTokenKey, token);
    settings.setValue(kUsernameKey, username);
    settings.setValue(kUserIdKey, userId);
}

void Authentication::clearSession() {
    QSettings settings("Uchat", "UchatApp");
    settings.remove(kTokenKey);
    settings.remove(kUsernameKey);
    settings.remove(kUserIdKey);
}

QString Authentication::getStoredToken() const {
    QSettings settings("Uchat", "UchatApp");
    return settings.value(kTokenKey).toString();
}

bool Authentication::checkExistingSession() {
    QString token = getStoredToken();
    if (token.isEmpty()) {
        return false;
    }
    return true;
}

Authentication::Authentication(QObject* parent)
    : QObject(parent), m_networkManager(new QNetworkAccessManager(this)) {
    connect(m_networkManager, &QNetworkAccessManager::finished, this,
            &Authentication::handleLoginResponse);
}

void Authentication::login(const QString& email, const QString& password) {
    const QString normalizedEmail = email.trimmed();

    if (normalizedEmail.isEmpty() || password.isEmpty()) {
        emit loginFailed(QStringLiteral("Email va parolni kiriting."));
        return;
    }

    QJsonObject payload;
    payload.insert(QStringLiteral("email"), normalizedEmail);
    payload.insert(QStringLiteral("password"), password);

    QNetworkRequest request(kLoginUrl);

    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    request.setRawHeader("Accept", "application/json");
    request.setTransferTimeout(15000);

    emit loginStarted();

    QNetworkReply* reply =
        m_networkManager->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));

    reply->setProperty("requestType", QStringLiteral("login"));
}

void Authentication::handleLoginResponse(QNetworkReply* reply) {
    if (!reply) {
        return;
    }

    if (reply->property("requestType").toString() != QStringLiteral("login")) {
        reply->deleteLater();
        return;
    }

    const int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    const auto networkError = reply->error();
    const QByteArray body = reply->readAll();

    QNetworkReply* finishedReply = reply;
    finishedReply->deleteLater();

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(body, &parseError);

    const QJsonObject root = document.isObject() ? document.object() : QJsonObject{};

    if (networkError != QNetworkReply::NoError) {
        if (statusCode == 401 || statusCode == 403) {
            emit loginFailed(QStringLiteral("Email yoki parol noto'g'ri."));
        } else if (statusCode >= 500) {
            emit loginFailed(QStringLiteral("Serverda xatolik. Keyinroq qayta urinib ko'ring."));
        } else if (statusCode == 0) {
            emit loginFailed(QStringLiteral("Serverga ulanib bo'lmadi. Internetni tekshiring."));
        } else {
            emit loginFailed(QStringLiteral("Login amalga oshmadi (HTTP %1).").arg(statusCode));
        }
        return;
    }

    if (statusCode < 200 || statusCode >= 300) {
        QString message = QStringLiteral("Login amalga oshmadi (HTTP %1).").arg(statusCode);

        const QString serverMessage = root.value(QStringLiteral("message")).toString();

        if (!serverMessage.isEmpty()) {
            message = serverMessage;
        }

        emit loginFailed(message);
        return;
    }

    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        emit loginFailed(QStringLiteral("Server noto'g'ri JSON javob qaytardi."));
        return;
    }

    const QString accessToken = root.value(QStringLiteral("access_token")).toString();

    const QString tokenType = root.value(QStringLiteral("token_type")).toString();

    const QJsonObject user = root.value(QStringLiteral("user")).toObject();

    const QString userId = user.value(QStringLiteral("id")).toString();

    const QString username = user.value(QStringLiteral("username")).toString();

    if (accessToken.isEmpty() || userId.isEmpty() || username.isEmpty()) {
        emit loginFailed(QStringLiteral("Server javobida kerakli ma'lumotlar mavjud emas."));
        return;
    }

    if (!tokenType.isEmpty() &&
        tokenType.compare(QStringLiteral("Bearer"), Qt::CaseInsensitive) != 0) {
        emit loginFailed(QStringLiteral("Noma'lum token turi."));
        return;
    }

    emit loginSucceeded(accessToken, username, userId);
}
