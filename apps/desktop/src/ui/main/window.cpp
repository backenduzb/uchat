
#include "window.hpp"

#include <QLabel>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("Uchat");
    resize(1100, 720);
    setMinimumSize(800, 500);

    auto* label = new QLabel("Welcome");
    label->setAlignment(Qt::AlignCenter);

    setCentralWidget(label);
}
