#include "ThemeManager.hpp"

#include <QApplication>
#include <QFile>
#include <QSettings>
#include <QStringList>
#include <QWidget>
#include <qlogging.h>

#include <cmath>

namespace {
const QStringList kGlobalStylePaths = {
    QStringLiteral(":/styles/global.qss"),
    QStringLiteral(":/styles/components/topbar.qss"),
    QStringLiteral(":/styles/components/drawer.qss"),
};

constexpr const char* kOrg = "Uchat";
constexpr const char* kApp = "UchatApp";
constexpr const char* kModeKey = "appearance/mode";
constexpr const char* kAccentKey = "appearance/accent";
constexpr const char* kCustomKey = "appearance/customAccent";

const QColor kDefaultAccent("#229ED9");

QColor validatedAccent(const QVariant& value, const QColor& fallback) {
    if (value.typeId() != QMetaType::QString) {
        return fallback;
    }
    const QColor color(value.toString());
    return ThemeManager::isValidAccent(color) ? color : fallback;
}
} // namespace

ThemeManager::ThemeManager(QObject* parent) : QObject(parent) {
    for (const QString& path : kGlobalStylePaths) {
        const QString part = loadStyleSheet(path);
        if (part.isEmpty()) {
            qWarning() << "ThemeManager: stylesheet could not be loaded:" << path;
            continue;
        }
        m_globalStyle += part + '\n';
    }

    if (m_globalStyle.isEmpty()) {
        qWarning("ThemeManager: global styles could not be loaded");
    }

    loadPersisted();
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

QColor ThemeManager::accentColor() const noexcept {
    return m_accent;
}

bool ThemeManager::setAccentColor(const QColor& color) {
    if (!isValidAccent(color) || color == m_accent) {
        return color == m_accent;
    }
    m_accent = color;
    if (!reloadStyles()) {
        return false;
    }
    savePersisted();
    emit accentChanged(m_accent);
    emit themeChanged(m_currentTheme);
    emit appearanceChanged();
    return true;
}

QColor ThemeManager::customAccent() const noexcept {
    return m_customAccent;
}

bool ThemeManager::setCustomAccent(const QColor& color) {
    if (!isValidAccent(color)) {
        return false;
    }
    m_customAccent = color;
    savePersisted();
    return setAccentColor(color);
}

ThemeManager::ThemePalette ThemeManager::currentPalette() const noexcept {
    return paletteFor(m_currentTheme, m_accent);
}

ThemeManager::ThemePalette ThemeManager::paletteFor(Theme theme) noexcept {
    return paletteFor(theme, kDefaultAccent);
}

ThemeManager::ThemePalette ThemeManager::paletteFor(Theme theme, const QColor& accent) noexcept {
    const QColor safeAccent = isValidAccent(accent) ? accent : kDefaultAccent;
    const QColor onAccent = contrastText(safeAccent);

    ThemePalette palette;
    if (theme == Theme::Light) {
        palette.inputText = QColor("#000000");
        palette.inputPlaceholder = QColor("#767676");
        palette.inputBorderNormal = QColor("#777777");
        palette.inputBorderFocused = safeAccent;
        palette.inputBorderDisabled = QColor("#BBBBBB");
        palette.selectionBackground = safeAccent;
        palette.selectionForeground = onAccent;
        palette.buttonBackground = safeAccent;
        palette.buttonHover = hoverFor(safeAccent);
        palette.buttonPressed = pressedFor(safeAccent);
        palette.buttonDisabled = QColor("#E5E5E5");
        palette.buttonText = onAccent;
        palette.buttonTextDisabled = QColor("#777777");
        palette.rippleColor = QColor(255, 255, 255, 70);
        palette.topBarBackground = QColor("#FFFFFF");
        palette.topBarBorder = QColor("#E5E5E5");
        palette.searchBackground = QColor("#F0F0F0");
        palette.searchBackgroundFocused = QColor("#E8E8E8");
        palette.searchText = QColor("#000000");
        palette.searchPlaceholder = QColor("#767676");
        palette.searchIcon = QColor("#767676");
        palette.iconColor = QColor("#555555");
        palette.iconRippleColor = QColor(0, 0, 0, 40);
        palette.badgeColor = QColor("#FF5C5C");
    } else {
        palette.inputText = QColor("#F5F5F5");
        palette.inputPlaceholder = QColor("#9A9A9A");
        palette.inputBorderNormal = QColor("#777777");
        palette.inputBorderFocused = safeAccent;
        palette.inputBorderDisabled = QColor("#4A4A4A");
        palette.selectionBackground = safeAccent;
        palette.selectionForeground = onAccent;
        palette.buttonBackground = safeAccent;
        palette.buttonHover = hoverFor(safeAccent);
        palette.buttonPressed = pressedFor(safeAccent);
        palette.buttonDisabled = QColor("#2A2A2A");
        palette.buttonText = onAccent;
        palette.buttonTextDisabled = QColor("#888888");
        palette.rippleColor = QColor(0, 0, 0, 45);
        palette.topBarBackground = QColor("#171717");
        palette.topBarBorder = QColor("#232323");
        palette.searchBackground = QColor("#2C2C2C");
        palette.searchBackgroundFocused = QColor("#333333");
        palette.searchText = QColor("#F5F5F5");
        palette.searchPlaceholder = QColor("#71717A");
        palette.searchIcon = QColor("#71717A");
        palette.iconColor = QColor("#A1A1AA");
        palette.iconRippleColor = QColor(255, 255, 255, 45);
        palette.badgeColor = QColor("#FF5C5C");
    }
    return palette;
}

ThemeManager::SurfaceColors ThemeManager::surfaceColors() const noexcept {
    return surfacesFor(m_currentTheme);
}

ThemeManager::SurfaceColors ThemeManager::surfacesFor(Theme theme) noexcept {
    SurfaceColors colors;
    if (theme == Theme::Light) {
        colors.background = QColor("#FFFFFF");
        colors.surface = QColor("#F4F4F5");
        colors.card = QColor("#FFFFFF");
        colors.field = QColor("#F0F0F0");
        colors.foreground = QColor("#09090B");
        colors.muted = QColor("#71717A");
        colors.stroke = QColor("#E4E4E7");
        colors.strokeSoft = QColor("#F0F0F2");
    } else {
        colors.background = QColor("#09090B");
        colors.surface = QColor("#131316");
        colors.card = QColor("#17171A");
        colors.field = QColor("#1F1F23");
        colors.foreground = QColor("#FAFAFA");
        colors.muted = QColor("#A1A1AA");
        colors.stroke = QColor("#2A2A30");
        colors.strokeSoft = QColor("#1F1F23");
    }
    return colors;
}

QVector<ThemeManager::AccentOption> ThemeManager::defaultAccents() noexcept {
    return {
        {QStringLiteral("telegram"), QStringLiteral("Telegram blue"), QColor("#229ED9")},
        {QStringLiteral("azure"), QStringLiteral("Azure blue"), QColor("#3B82F6")},
        {QStringLiteral("violet"), QStringLiteral("Violet"), QColor("#7C6CF0")},
        {QStringLiteral("purple"), QStringLiteral("Purple"), QColor("#A855F7")},
        {QStringLiteral("emerald"), QStringLiteral("Emerald green"), QColor("#16A34A")},
        {QStringLiteral("teal"), QStringLiteral("Teal"), QColor("#0D9488")},
        {QStringLiteral("amber"), QStringLiteral("Amber"), QColor("#D97706")},
        {QStringLiteral("coral"), QStringLiteral("Coral"), QColor("#EA5A3E")},
        {QStringLiteral("rose"), QStringLiteral("Rose"), QColor("#E11D48")},
        {QStringLiteral("slate"), QStringLiteral("Slate gray"), QColor("#64748B")},
    };
}

QColor ThemeManager::contrastText(const QColor& background) noexcept {
    if (!background.isValid()) {
        return QColor("#FFFFFF");
    }
    const auto linear = [](int channel) {
        const double c = static_cast<double>(channel) / 255.0;
        return c <= 0.03928 ? c / 12.92 : std::pow((c + 0.055) / 1.055, 2.4);
    };
    const double luminance =
        0.2126 * linear(background.red()) + 0.7152 * linear(background.green()) +
        0.0722 * linear(background.blue());
    return luminance > 0.35 ? QColor("#09090B") : QColor("#FFFFFF");
}

bool ThemeManager::isValidAccent(const QColor& color) noexcept {
    return color.isValid() && color.alpha() == 255;
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

    savePersisted();
    emit themeChanged(m_currentTheme);
    emit appearanceChanged();
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

QString ThemeManager::accentStyleSheet() const {
    const QColor accent = isValidAccent(m_accent) ? m_accent : kDefaultAccent;
    const QColor hover = hoverFor(accent);
    const QColor pressed = pressedFor(accent);
    const QColor onAccent = contrastText(accent);
    const QString accentName = accent.name();
    const QString hoverName = hover.name();
    const QString pressedName = pressed.name();
    const QString onAccentName = onAccent.name();

    return QStringLiteral(
        "QLineEdit { selection-background-color: %1; selection-color: %2; }\n"
        "InputField { qproperty-focusedBorderColor: %1; }\n"
        "QPushButton { background-color: %1; color: %2; border-color: %1; }\n"
        "QPushButton:hover { background-color: %3; color: %2; }\n"
        "QPushButton:pressed { background-color: %4; color: %2; }\n"
        "Button { color: %2; qproperty-normalBackground: %1; "
        "qproperty-hoverBackground: %3; qproperty-pressedBackground: %4; }\n"
        "TopBar QLineEdit#topBarSearchInput { selection-background-color: %1; "
        "selection-color: %2; }\n")
        .arg(accentName, onAccentName, hoverName, pressedName);
}

bool ThemeManager::reloadStyles() {
    const QString themeStyle = loadStyleSheet(themePath(m_currentTheme));

    if (themeStyle.isNull() || themeStyle.isEmpty()) {
        return false;
    }
    if (qApp) {
        qApp->setStyleSheet(m_globalStyle + "\n" + themeStyle + "\n" + accentStyleSheet());
    }

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

void ThemeManager::loadPersisted() {
    QSettings settings(kOrg, kApp);
    const QString mode = settings.value(kModeKey, QStringLiteral("light")).toString();
    m_currentTheme = (mode.compare(QStringLiteral("dark"), Qt::CaseInsensitive) == 0)
        ? Theme::Dark
        : Theme::Light;
    m_accent = validatedAccent(settings.value(kAccentKey), kDefaultAccent);
    const QVariant customValue = settings.value(kCustomKey);
    if (customValue.typeId() == QMetaType::QString) {
        const QColor custom(customValue.toString());
        if (isValidAccent(custom)) {
            m_customAccent = custom;
        }
    }
}

void ThemeManager::savePersisted() const {
    QSettings settings(kOrg, kApp);
    settings.setValue(kModeKey, m_currentTheme == Theme::Dark ? QStringLiteral("dark")
                                                             : QStringLiteral("light"));
    settings.setValue(kAccentKey, m_accent.name());
    if (isValidAccent(m_customAccent)) {
        settings.setValue(kCustomKey, m_customAccent.name());
    }
}

QColor ThemeManager::hoverFor(const QColor& accent) noexcept {
    if (!accent.isValid()) {
        return accent;
    }
    int h = 0;
    int s = 0;
    int v = 0;
    accent.getHsv(&h, &s, &v);
    return v > 150 ? accent.darker(110) : accent.lighter(112);
}

QColor ThemeManager::pressedFor(const QColor& accent) noexcept {
    if (!accent.isValid()) {
        return accent;
    }
    int h = 0;
    int s = 0;
    int v = 0;
    accent.getHsv(&h, &s, &v);
    return v > 150 ? accent.darker(122) : accent.lighter(125);
}
