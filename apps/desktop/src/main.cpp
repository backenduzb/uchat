
#include <QApplication>
#include <QObject>

#include "app/AppState.hpp"
#include "ui/auth/login.hpp"
#include "ui/main/window.hpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("Uchat");
    QApplication::setApplicationVersion("0.1.0");

    AppState state;

    LoginWindow loginWindow;
    MainWindow mainWindow;

    QObject::connect(
        &loginWindow,
        &LoginWindow::loginRequested,
        &app,
        [&state](const QString& username, const QString& password) {
            Q_UNUSED(username);
            Q_UNUSED(password);

            state.setStatus(AppStatus::Authenticated);
        }
    );

    QObject::connect(
        &state,
        &AppState::statusChanged,
        &app,
        [&loginWindow, &mainWindow](AppStatus status) {
            if (status == AppStatus::Authenticated) {
                loginWindow.hide();
                mainWindow.show();
            } else {
                mainWindow.hide();
                loginWindow.show();
            }
        }
    );

    loginWindow.show();

    return app.exec();
}
