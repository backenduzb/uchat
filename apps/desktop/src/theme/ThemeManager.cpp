#include "ThemeManager.hpp"

#include <QApplication>
#include <QFile>
#include <QWidget>
#include <qlogging.h>
#include <qpicture.h>

ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {
    m_globalStyle = loadStyleSheet(":/styles/global.qss");

    if (m_globalStyle.isEmpty()) {
        qWarning("ThemeManager: global.css could not loaded");
    }

    reloadStyles();
}

QString ThemeManager::loadStyleSheet(const QString& path) const {
    QFile file(path);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot load stylesheet: " << path;
        return {};
    }

    return QString::fromUtf8(file.readAll());
}

QString ThemeManager::themePath(Theme theme) const {
    switch (theme) {
    case Theme::Dark:
        return ":/styles/themes/dark.qss";
    case Theme::Light:
        return ":/styles/themes/light.qss";
    }

    return {};
}

ThemeManager::Theme ThemeManager::currentTheme() const noexcept {
    return m_currentTheme;
}

ThemeManager::ThemePalette ThemeManager::currentPalette() const noexcept {
    return paletteFor(m_currentTheme);
}

ThemeManager::ThemePalette ThemeManager::paletteFor(Theme theme) noexcept {
    ThemePalette palette;
    if (theme == Theme::Light) {
        palette.inputText = QColor("#000000");
        palette.inputPlaceholder = QColor("#767676");
        palette.inputBorderNormal = QColor("#777777");
        palette.inputBorderFocused = QColor("#000000");
        palette.inputBorderDisabled = QColor("#BBBBBB");
        palette.selectionBackground = QColor("#000000");
        palette.selectionForeground = QColor("#FFFFFF");
        palette.buttonBackground = QColor("#000000");
        palette.buttonHover = QColor("#333333");
        palette.buttonPressed = QColor("#555555");
        palette.buttonDisabled = QColor("#E5E5E5");
        palette.buttonText = QColor("#FFFFFF");
        palette.buttonTextDisabled = QColor("#777777");
    } else {
        palette.inputText = QColor("#F5F5F5");
        palette.inputPlaceholder = QColor("#9A9A9A");
        palette.inputBorderNormal = QColor("#777777");
        palette.inputBorderFocused = QColor("#FFFFFF");
        palette.inputBorderDisabled = QColor("#4A4A4A");
        palette.selectionBackground = QColor("#FFFFFF");
        palette.selectionForeground = QColor("#171717");
        palette.buttonBackground = QColor("#F5F5F5");
        palette.buttonHover = QColor("#D8D8D8");
        palette.buttonPressed = QColor("#BBBBBB");
        palette.buttonDisabled = QColor("#2A2A2A");
        palette.buttonText = QColor("#171717");
        palette.buttonTextDisabled = QColor("#888888");
    }
    return palette;
}

bool ThemeManager::setTheme(Theme theme) {
    if (theme == m_currentTheme) {
        return true;
    }

    const Theme previousTheme = m_currentTheme;
    m_currentTheme = theme;

    if (!reloadStyles()) {
        m_currentTheme = previousTheme;
        reloadStyles();
        return false;
    }

    emit themeChanged(m_currentTheme);
    return true;
}

bool ThemeManager::registerPage(QWidget* page, const QString& stylesheetPath) {
    if (!page) {
        return false;
    }

    const QString pageStyles = loadStyleSheet(stylesheetPath);
    if (pageStyles.isNull() || pageStyles.isEmpty()) {
        return false;
    }

    for (auto& entry : m_pages) {
        if (entry.page == page) {
            entry.stylesheetPath = stylesheetPath;
            return reloadStyles();
        }
    }

    m_pages.push_back({page, stylesheetPath});

    if (!reloadStyles()) {
        m_pages.removeLast();
        return false;
    }

    return true;
}

bool ThemeManager::reloadStyles() {
    const QString themeStyle = loadStyleSheet(themePath(m_currentTheme));

    if (themeStyle.isNull() || themeStyle.isEmpty()) {
        return false;
    }
    qApp->setStyleSheet(m_globalStyle + "\n" + themeStyle);

    for (auto it = m_pages.begin(); it != m_pages.end();) {
        if (!it->page) {
            it = m_pages.erase(it);
            continue;
        }

        const QString pageStyle = loadStyleSheet(it->stylesheetPath);

        if (pageStyle.isNull() || pageStyle.isEmpty()) {
            return false;
        }

        it->page->setStyleSheet(pageStyle);
        ++it;
    }

    return true;
}
