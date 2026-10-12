#pragma once

#include <QColor>
#include <QObject>
#include <QString>
#include <QVector>
#include <QPointer>

class QWidget;

class ThemeManager final : public QObject {
    Q_OBJECT

    public:
    enum class Theme {
        Dark,
        Light,
    };
    Q_ENUM(Theme)

    struct ThemePalette {
        QColor inputText;
        QColor inputPlaceholder;
        QColor inputBorderNormal;
        QColor inputBorderFocused;
        QColor inputBorderDisabled;
        QColor selectionBackground;
        QColor selectionForeground;
        QColor buttonBackground;
        QColor buttonHover;
        QColor buttonPressed;
        QColor buttonDisabled;
        QColor buttonText;
        QColor buttonTextDisabled;
    };

    explicit ThemeManager(QObject *parent = nullptr);

    bool setTheme(Theme theme);
    Theme currentTheme() const noexcept;

    [[nodiscard]] ThemePalette currentPalette() const noexcept;
    [[nodiscard]] static ThemePalette paletteFor(Theme theme) noexcept;

    bool registerPage(
        QWidget *page,
        const QString &styleshettPath
    );

    signals:
    void themeChanged(Theme theme);
    private:
        struct PageStyle {
            QPointer<QWidget> page;
            QString stylesheetPath;
        };

        QString loadStyleSheet(const QString& path) const;
        QString themePath(Theme theme) const;
        bool reloadStyles();

        Theme m_currentTheme = Theme::Light;
        QString m_globalStyle;
        QVector<PageStyle> m_pages;
};