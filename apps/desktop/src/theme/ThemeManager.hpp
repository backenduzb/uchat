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

    struct AccentOption {
        QString id;
        QString name;
        QColor color;
    };

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
        QColor rippleColor;
        QColor topBarBackground;
        QColor topBarBorder;
        QColor searchBackground;
        QColor searchBackgroundFocused;
        QColor searchText;
        QColor searchPlaceholder;
        QColor searchIcon;
        QColor iconColor;
        QColor iconRippleColor;
        QColor badgeColor;
    };

    struct SurfaceColors {
        QColor background;
        QColor surface;
        QColor card;
        QColor field;
        QColor foreground;
        QColor muted;
        QColor stroke;
        QColor strokeSoft;
    };

    explicit ThemeManager(QObject *parent = nullptr);

    bool setTheme(Theme theme);
    Theme currentTheme() const noexcept;

    QColor accentColor() const noexcept;
    bool setAccentColor(const QColor &color);
    QColor customAccent() const noexcept;
    bool setCustomAccent(const QColor &color);

    [[nodiscard]] ThemePalette currentPalette() const noexcept;
    [[nodiscard]] static ThemePalette paletteFor(Theme theme, const QColor &accent) noexcept;
    [[nodiscard]] static ThemePalette paletteFor(Theme theme) noexcept;
    [[nodiscard]] SurfaceColors surfaceColors() const noexcept;
    [[nodiscard]] static SurfaceColors surfacesFor(Theme theme) noexcept;

    [[nodiscard]] static QVector<AccentOption> defaultAccents() noexcept;
    [[nodiscard]] static QColor contrastText(const QColor &background) noexcept;
    [[nodiscard]] static bool isValidAccent(const QColor &color) noexcept;

    bool registerPage(
        QWidget *page,
        const QString &styleshettPath
    );

    signals:
    void themeChanged(Theme theme);
    void accentChanged(const QColor &accent);
    void appearanceChanged();
    private:
        struct PageStyle {
            QPointer<QWidget> page;
            QString stylesheetPath;
        };

        QString loadStyleSheet(const QString& path) const;
        QString themePath(Theme theme) const;
        QString accentStyleSheet() const;
        bool reloadStyles();
        void loadPersisted();
        void savePersisted() const;
        static QColor hoverFor(const QColor &accent) noexcept;
        static QColor pressedFor(const QColor &accent) noexcept;

        Theme m_currentTheme = Theme::Light;
        QColor m_accent = QColor("#229ED9");
        QColor m_customAccent;
        QString m_globalStyle;
        QVector<PageStyle> m_pages;
};
