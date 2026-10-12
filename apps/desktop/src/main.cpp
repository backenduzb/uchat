#include <QApplication>
#include <QDebug>

#include "animation/AnimationManager.hpp"
#include "theme/ThemeManager.hpp"
#include "ui/main/window.hpp"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("Uchat");
    QApplication::setApplicationVersion("0.1.0");

    ThemeManager themeManager;
    AnimationManager animationManager;

    MainWindow mainWindow(&themeManager, &animationManager);
    mainWindow.show();

    return app.exec();
}