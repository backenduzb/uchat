#include "TopBar.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPaintEvent>
#include <QPainter>

#include "animation/AnimationManager.hpp"
#include "components/IconButton.hpp"
#include "components/Lucide.hpp"
#include "theme/ThemeManager.hpp"

TopBar::TopBar(ThemeManager* themeManager, AnimationManager* animationManager, QWidget* parent)
    : QWidget(parent), m_themeManager(themeManager), m_animationManager(animationManager) {
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedHeight(56);

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(8, 8, 12, 8);
    layout->setSpacing(8);

    m_menuButton = new IconButton(Lucide::Icon::Menu, this, m_themeManager);
    m_menuButton->setObjectName("topBarMenuButton");
    m_menuButton->setFixedSize(40, 40);
    m_menuButton->setIconSize(22);
    m_menuButton->setShowBadge(true);
    m_menuButton->setToolTip("Menu");
    connect(m_menuButton, &QAbstractButton::clicked, this, &TopBar::menuClicked);
    layout->addWidget(m_menuButton);

    m_searchBox = new QWidget(this);
    m_searchBox->setObjectName("topBarSearchBox");
    m_searchBox->setAttribute(Qt::WA_StyledBackground, true);
    m_searchBox->setStyleSheet("background: transparent; border: none;");
    m_searchBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_searchBox->setFixedHeight(40);

    auto* searchLayout = new QHBoxLayout(m_searchBox);
    searchLayout->setContentsMargins(0, 0, 4, 0);
    searchLayout->setSpacing(6);

    m_searchIconLabel = new QLabel(m_searchBox);
    m_searchIconLabel->setObjectName("topBarSearchIcon");
    m_searchIconLabel->setFixedSize(36, 24);
    m_searchIconLabel->setAlignment(Qt::AlignCenter);
    searchLayout->addWidget(m_searchIconLabel);

    m_searchInput = new QLineEdit(m_searchBox);
    m_searchInput->setObjectName("topBarSearchInput");
    m_searchInput->setPlaceholderText("Search");
    m_searchInput->setClearButtonEnabled(false);
    m_searchInput->setFrame(false);
    m_searchInput->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_searchInput->installEventFilter(this);
    searchLayout->addWidget(m_searchInput, 1);

    m_clearButton = new IconButton(Lucide::Icon::X, m_searchBox, m_themeManager);
    m_clearButton->setObjectName("topBarClearButton");
    m_clearButton->setFixedSize(28, 28);
    m_clearButton->setIconSize(14);
    m_clearButton->setVisible(false);
    m_clearButton->setToolTip("Clear");
    connect(m_clearButton, &QAbstractButton::clicked, this, [this]() {
        m_searchInput->clear();
        m_searchInput->setFocus();
    });
    searchLayout->addWidget(m_clearButton);

    layout->addWidget(m_searchBox, 1);

    m_createButton = new IconButton(Lucide::Icon::Plus, this, m_themeManager);
    m_createButton->setObjectName("topBarCreateButton");
    m_createButton->setFixedSize(40, 40);
    m_createButton->setIconSize(22);
    m_createButton->setToolTip("Create group");
    m_createButton->setAccessibleName("Create group");
    connect(m_createButton, &QAbstractButton::clicked, this, &TopBar::createGroupRequested);
    layout->addWidget(m_createButton);

    connect(m_searchInput, &QLineEdit::textChanged, this, [this](const QString& text) {
        m_clearButton->setVisible(!text.isEmpty());
        emit searchTextChanged(text);
    });
    connect(m_searchInput, &QLineEdit::returnPressed, this,
            [this]() { emit searchSubmitted(m_searchInput->text()); });

    if (m_themeManager) {
        connect(m_themeManager, &ThemeManager::themeChanged, this,
                [this]() { refreshFromTheme(true); });
        refreshFromTheme(false);
    }
    updateSearchIcon();
}

void TopBar::setShowMenuBadge(bool show) {
    if (m_menuButton) {
        m_menuButton->setShowBadge(show);
    }
}

bool TopBar::showMenuBadge() const noexcept {
    return m_menuButton ? m_menuButton->showBadge() : false;
}

QString TopBar::searchText() const {
    return m_searchInput ? m_searchInput->text() : QString();
}

void TopBar::setSearchText(const QString& text) {
    if (m_searchInput) {
        m_searchInput->setText(text);
    }
}

void TopBar::setSearchPlaceholder(const QString& text) {
    if (m_searchInput) {
        m_searchInput->setPlaceholderText(text);
    }
}

IconButton* TopBar::menuButton() const noexcept {
    return m_menuButton;
}

QLineEdit* TopBar::searchInput() const noexcept {
    return m_searchInput;
}

bool TopBar::eventFilter(QObject* watched, QEvent* event) {
    if (watched == m_searchInput) {
        if (event->type() == QEvent::FocusIn) {
            setSearchFocused(true);
        } else if (event->type() == QEvent::FocusOut) {
            setSearchFocused(false);
        }
    }
    return QWidget::eventFilter(watched, event);
}

void TopBar::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(m_currentTopBar);
    painter.drawRect(rect());

    if (m_themeManager) {
        painter.setBrush(m_themeManager->currentPalette().topBarBorder);
        painter.drawRect(0, height() - 1, width(), 1);
    }

    if (m_searchBox) {
        const QRect boxRect = m_searchBox->geometry();
        painter.setBrush(m_currentSearchBg);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.drawRoundedRect(QRectF(boxRect), 20.0, 20.0);
        painter.setRenderHint(QPainter::Antialiasing, false);
    }
}

void TopBar::refreshFromTheme(bool animate) {
    if (!m_themeManager) {
        return;
    }
    const auto palette = m_themeManager->currentPalette();
    m_topBarBackground = palette.topBarBackground;
    m_searchBackground =
        m_searchFocused ? palette.searchBackgroundFocused : palette.searchBackground;

    if (m_searchInput) {
        QPalette pal = m_searchInput->palette();
        pal.setColor(QPalette::Text, palette.searchText);
        pal.setColor(QPalette::PlaceholderText, palette.searchPlaceholder);
        m_searchInput->setPalette(pal);
    }

    updateSearchIcon();

    if (animate) {
        startBackgroundAnimation(m_topBarBackground, m_searchBackground);
    } else {
        AnimationManager::stopAnimation(m_bgAnimation);
        AnimationManager::stopAnimation(m_searchAnimation);
        m_currentTopBar = m_topBarBackground;
        m_currentSearchBg = m_searchBackground;
        update();
    }
}

void TopBar::startBackgroundAnimation(const QColor& topBarTarget, const QColor& searchTarget) {
    AnimationManager::stopAnimation(m_bgAnimation);
    AnimationManager::stopAnimation(m_searchAnimation);

    const QColor topFrom = m_currentTopBar;
    const QColor searchFrom = m_currentSearchBg;

    m_bgAnimation = AnimationManager::animateColorValue(
        topFrom, topBarTarget, 200, this, [this](const QColor& color) {
            m_currentTopBar = color;
            update();
        });
    m_searchAnimation = AnimationManager::animateColorValue(
        searchFrom, searchTarget, 180, this, [this](const QColor& color) {
            m_currentSearchBg = color;
            update();
        });
}

void TopBar::updateSearchIcon() {
    if (!m_searchIconLabel || !m_themeManager) {
        return;
    }
    const QColor iconColor = m_themeManager->currentPalette().searchIcon;
    const QPixmap pm = Lucide::pixmap(Lucide::Icon::Search, 18, iconColor, 2.0,
                                      devicePixelRatioF());
    m_searchIconLabel->setPixmap(pm);
}

void TopBar::setSearchFocused(bool focused) {
    if (m_searchFocused == focused) {
        return;
    }
    m_searchFocused = focused;
    if (!m_themeManager) {
        return;
    }
    const auto palette = m_themeManager->currentPalette();
    const QColor target =
        focused ? palette.searchBackgroundFocused : palette.searchBackground;
    m_searchBackground = target;

    AnimationManager::stopAnimation(m_searchAnimation);
    const QColor from = m_currentSearchBg;
    m_searchAnimation = AnimationManager::animateColorValue(
        from, target, 150, this, [this](const QColor& color) {
            m_currentSearchBg = color;
            update();
        });
}
