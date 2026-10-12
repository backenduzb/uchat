#pragma once

#include <QColor>
#include <QPointer>
#include <QWidget>

class AnimationManager;
class IconButton;
class QLabel;
class QLineEdit;
class ThemeManager;
class QVariantAnimation;

class TopBar final : public QWidget {
    Q_OBJECT

public:
    explicit TopBar(ThemeManager* themeManager, AnimationManager* animationManager,
                    QWidget* parent = nullptr);

    void setShowMenuBadge(bool show);
    [[nodiscard]] bool showMenuBadge() const noexcept;

    [[nodiscard]] QString searchText() const;
    void setSearchText(const QString& text);
    void setSearchPlaceholder(const QString& text);

    IconButton* menuButton() const noexcept;
    QLineEdit* searchInput() const noexcept;

signals:
    void menuClicked();
    void createGroupRequested();
    void searchTextChanged(const QString& text);
    void searchSubmitted(const QString& text);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void refreshFromTheme(bool animate);
    void startBackgroundAnimation(const QColor& topBarTarget, const QColor& searchTarget);
    void updateSearchIcon();
    void setSearchFocused(bool focused);

    ThemeManager* m_themeManager = nullptr;
    AnimationManager* m_animationManager = nullptr;

    IconButton* m_menuButton = nullptr;
    QWidget* m_searchBox = nullptr;
    QLabel* m_searchIconLabel = nullptr;
    QLineEdit* m_searchInput = nullptr;
    IconButton* m_clearButton = nullptr;
    IconButton* m_createButton = nullptr;

    QColor m_topBarBackground = QColor("#171717");
    QColor m_searchBackground = QColor("#2C2C2C");
    QColor m_currentTopBar = QColor("#171717");
    QColor m_currentSearchBg = QColor("#2C2C2C");
    bool m_searchFocused = false;

    QPointer<QVariantAnimation> m_bgAnimation;
    QPointer<QVariantAnimation> m_searchAnimation;
};
