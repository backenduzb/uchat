#pragma once

#include <QAbstractButton>
#include <QColor>
#include <QWidget>

#include "theme/ThemeManager.hpp"

class QButtonGroup;
class QLabel;
class QGridLayout;
class AnimationManager;
class IconButton;

class ThemePreviewCard final : public QAbstractButton {
    Q_OBJECT

public:
    explicit ThemePreviewCard(
        ThemeManager* themeManager,
        ThemeManager::Theme mode,
        QWidget* parent = nullptr);

    [[nodiscard]] ThemeManager::Theme mode() const noexcept;
    [[nodiscard]] QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void refresh();

private:
    void updateCardStyle();

    ThemeManager* m_themeManager = nullptr;
    ThemeManager::Theme m_mode = ThemeManager::Theme::Dark;
};

class AccentSwatch final : public QAbstractButton {
    Q_OBJECT

public:
    explicit AccentSwatch(
        const QColor& color,
        const QString& name,
        bool isCustomSlot,
        QWidget* parent = nullptr);

    void setSwatchColor(const QColor& color);
    [[nodiscard]] QColor swatchColor() const noexcept;
    [[nodiscard]] QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QColor m_color;
    bool m_isCustomSlot = false;
};

class ThemeEditor final : public QWidget {
    Q_OBJECT

public:
    explicit ThemeEditor(ThemeManager* themeManager, QWidget* parent = nullptr);

    void refresh();

signals:
    void closeRequested();

private slots:
    void applySelectedMode();
    void applySelectedAccent();
    void chooseCustomColor();

private:
    void refreshTheme();
    void syncSelection();
    void updateCustomSlot();

    ThemeManager* m_themeManager = nullptr;

    IconButton* m_closeButton = nullptr;
    QLabel* m_titleLabel = nullptr;
    QLabel* m_subtitleLabel = nullptr;
    QLabel* m_modeSectionLabel = nullptr;
    QLabel* m_accentSectionLabel = nullptr;
    QLabel* m_hintLabel = nullptr;
    QWidget* m_separator = nullptr;

    ThemePreviewCard* m_darkCard = nullptr;
    ThemePreviewCard* m_lightCard = nullptr;
    QButtonGroup* m_modeGroup = nullptr;

    QGridLayout* m_accentGrid = nullptr;
    QVector<AccentSwatch*> m_swatches;
    AccentSwatch* m_customSwatch = nullptr;
    QButtonGroup* m_accentGroup = nullptr;
};

class ThemeEditorOverlay final : public QWidget {
    Q_OBJECT

public:
    explicit ThemeEditorOverlay(
        ThemeManager* themeManager,
        AnimationManager* animationManager,
        QWidget* parent = nullptr);

    [[nodiscard]] bool isEditorOpen() const noexcept;

public slots:
    void openEditor();
    void closeEditor();

signals:
    void editorClosed();

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private:
    void fitToParent();
    void constrainEditor();

    ThemeManager* m_themeManager = nullptr;
    AnimationManager* m_animationManager = nullptr;
    ThemeEditor* m_editor = nullptr;
    bool m_open = false;
};
