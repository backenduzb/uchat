#include "MenuDrawer.hpp"

#include <QEvent>
#include <QAction>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPropertyAnimation>
#include <QVBoxLayout>

#include "animation/AnimationManager.hpp"
#include "components/Button.hpp"
#include "components/IconButton.hpp"
#include "components/Lucide.hpp"
#include "theme/ThemeManager.hpp"

namespace {
constexpr int kAnimationMs = 260;

void stopPanelAnimations(QWidget* panel, QObject* owner) {
    if (!panel || !owner) {
        return;
    }
    const auto animations = owner->findChildren<QPropertyAnimation*>();
    for (QPropertyAnimation* animation : animations) {
        if (animation && animation->targetObject() == panel) {
            animation->stop();
        }
    }
}
}

MenuDrawer::MenuDrawer(ThemeManager* themeManager, AnimationManager* animationManager,
                       QWidget* parent)
    : QWidget(parent), m_themeManager(themeManager), m_animationManager(animationManager) {
    setObjectName("menuDrawer");
    setAttribute(Qt::WA_StyledBackground, true);
    setVisible(false);
    setFocusPolicy(Qt::StrongFocus);

    auto* escAction = new QAction(this);
    escAction->setShortcut(Qt::Key_Escape);
    escAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(escAction, &QAction::triggered, this, &MenuDrawer::close);
    addAction(escAction);

    m_dim = new QWidget(this);
    m_dim->setObjectName("menuDim");
    m_dim->setAttribute(Qt::WA_StyledBackground, true);
    m_dim->installEventFilter(this);
    auto* dimEffect = new QGraphicsOpacityEffect(m_dim);
    dimEffect->setOpacity(0.0);
    m_dim->setGraphicsEffect(dimEffect);

    m_panel = new QWidget(this);
    m_panel->setObjectName("menuPanel");
    m_panel->setAttribute(Qt::WA_StyledBackground, true);
    m_panel->setFixedWidth(kPanelWidth);

    auto* panelLayout = new QVBoxLayout(m_panel);
    panelLayout->setContentsMargins(16, 12, 16, 16);
    panelLayout->setSpacing(0);

    auto* topRow = new QHBoxLayout();
    topRow->setContentsMargins(0, 0, 0, 0);
    topRow->setSpacing(8);

    m_logoutButton = new IconButton(Lucide::Icon::LogOut, m_panel, m_themeManager);
    m_logoutButton->setObjectName("menuLogoutButton");
    m_logoutButton->setFixedSize(36, 36);
    m_logoutButton->setIconSize(19);
    m_logoutButton->setToolTip("Logout");
    connect(m_logoutButton, &QAbstractButton::clicked, this, &MenuDrawer::logoutRequested);
    topRow->addWidget(m_logoutButton, 0, Qt::AlignLeft);

    topRow->addStretch(1);

    m_closeButton = new IconButton(Lucide::Icon::X, m_panel, m_themeManager);
    m_closeButton->setObjectName("menuCloseButton");
    m_closeButton->setFixedSize(36, 36);
    m_closeButton->setIconSize(18);
    m_closeButton->setToolTip("Close");
    connect(m_closeButton, &QAbstractButton::clicked, this, &MenuDrawer::close);
    topRow->addWidget(m_closeButton, 0, Qt::AlignRight);
    panelLayout->addLayout(topRow);

    m_avatarLabel = new QLabel(m_panel);
    m_avatarLabel->setObjectName("menuAvatar");
    m_avatarLabel->setFixedSize(64, 64);
    m_avatarLabel->setAlignment(Qt::AlignCenter);
    panelLayout->addSpacing(12);
    panelLayout->addWidget(m_avatarLabel, 0, Qt::AlignLeft);

    m_nameLabel = new QLabel(QStringLiteral("User"), m_panel);
    m_nameLabel->setObjectName("menuName");
    panelLayout->addSpacing(10);
    panelLayout->addWidget(m_nameLabel);

    m_usernameLabel = new QLabel(QStringLiteral("@username"), m_panel);
    m_usernameLabel->setObjectName("menuUsername");
    panelLayout->addWidget(m_usernameLabel);
    panelLayout->addSpacing(12);

    auto* separator = new QWidget(m_panel);
    separator->setObjectName("menuSeparator");
    separator->setFixedHeight(1);
    separator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    panelLayout->addWidget(separator);
    panelLayout->addSpacing(8);

    m_editThemeButton = new Button(QStringLiteral("Edit Theme"), m_panel, m_themeManager);
    m_editThemeButton->setObjectName("menuItemButton");
    m_editThemeButton->setCursor(Qt::PointingHandCursor);
    connect(m_editThemeButton, &QPushButton::clicked, this, [this]() {
        close();
        emit editThemeRequested();
    });
    panelLayout->addWidget(m_editThemeButton);
    panelLayout->addStretch(1);

    if (m_themeManager) {
        connect(m_themeManager, &ThemeManager::themeChanged, this,
                [this]() { refreshThemeIcon(); });
    }
    refreshThemeIcon();
    refreshUserLabels();

    if (parent) {
        parent->installEventFilter(this);
        fitToParent();
    }
}

void MenuDrawer::setUserInfo(const QString& displayName, const QString& username) {
    m_displayName = displayName.trimmed();
    m_username = username.trimmed();
    if (m_displayName.isEmpty()) {
        m_displayName = m_username.isEmpty() ? QStringLiteral("User") : m_username;
    }
    refreshUserLabels();
}

bool MenuDrawer::isOpen() const noexcept {
    return m_open;
}

void MenuDrawer::open() {
    if (m_open || !parentWidget()) {
        return;
    }
    stopPanelAnimations(m_panel, this);
    fitToParent();
    m_open = true;

    setVisible(true);
    raise();
    m_panel->raise();
    setFocus(Qt::PopupFocusReason);

    const int h = height();
    m_dim->setGeometry(rect());
    m_panel->setGeometry(-kPanelWidth, 0, kPanelWidth, h);

    auto* dimEffect = qobject_cast<QGraphicsOpacityEffect*>(m_dim->graphicsEffect());
    if (dimEffect) {
        auto* fade = new QPropertyAnimation(dimEffect, "opacity", this);
        fade->setDuration(kAnimationMs);
        fade->setStartValue(0.0);
        fade->setEndValue(1.0);
        fade->setEasingCurve(QEasingCurve::OutCubic);
        fade->start(QAbstractAnimation::DeleteWhenStopped);
    }

    m_animating = true;
    auto* slide = new QPropertyAnimation(m_panel, "pos", this);
    slide->setDuration(kAnimationMs);
    slide->setStartValue(QPoint(-kPanelWidth, 0));
    slide->setEndValue(QPoint(0, 0));
    slide->setEasingCurve(QEasingCurve::OutCubic);
    connect(slide, &QPropertyAnimation::finished, this, [this]() { m_animating = false; });
    slide->start(QAbstractAnimation::DeleteWhenStopped);

    emit drawerOpened();
}

void MenuDrawer::close() {
    if (!m_open) {
        return;
    }
    stopPanelAnimations(m_panel, this);
    m_animating = true;

    auto* dimEffect = qobject_cast<QGraphicsOpacityEffect*>(m_dim->graphicsEffect());
    if (dimEffect) {
        auto* fade = new QPropertyAnimation(dimEffect, "opacity", this);
        fade->setDuration(220);
        fade->setStartValue(dimEffect->opacity());
        fade->setEndValue(0.0);
        fade->setEasingCurve(QEasingCurve::InCubic);
        fade->start(QAbstractAnimation::DeleteWhenStopped);
    }

    auto* slide = new QPropertyAnimation(m_panel, "pos", this);
    slide->setDuration(220);
    slide->setStartValue(m_panel->pos());
    slide->setEndValue(QPoint(-kPanelWidth, 0));
    slide->setEasingCurve(QEasingCurve::InCubic);
    connect(slide, &QPropertyAnimation::finished, this, [this]() {
        m_animating = false;
        m_open = false;
        setVisible(false);
        emit drawerClosed();
    });
    slide->start(QAbstractAnimation::DeleteWhenStopped);
}

void MenuDrawer::toggle() {
    if (m_open) {
        close();
    } else {
        open();
    }
}

bool MenuDrawer::eventFilter(QObject* watched, QEvent* event) {
    if (watched == parentWidget()) {
        if (event->type() == QEvent::Resize) {
            fitToParent();
        }
    } else if (watched == m_dim) {
        if (event->type() == QEvent::MouseButtonPress) {
            close();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void MenuDrawer::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape && m_open) {
        close();
        event->accept();
        return;
    }
    QWidget::keyPressEvent(event);
}

void MenuDrawer::fitToParent() {
    QWidget* p = parentWidget();
    if (!p) {
        return;
    }
    setGeometry(p->rect());
    m_dim->setGeometry(rect());
    if (m_open && !m_animating) {
        m_panel->setGeometry(0, 0, kPanelWidth, height());
    } else if (!m_open) {
        m_panel->setGeometry(-kPanelWidth, 0, kPanelWidth, height());
    }
}

void MenuDrawer::refreshUserLabels() {
    if (!m_avatarLabel || !m_nameLabel || !m_usernameLabel) {
        return;
    }
    const QString initial =
        m_displayName.isEmpty() ? QStringLiteral("U") : m_displayName.left(1).toUpper();
    m_avatarLabel->setText(initial);
    m_nameLabel->setText(m_displayName.isEmpty() ? QStringLiteral("User") : m_displayName);
    const QString uname = m_username.isEmpty() ? m_displayName : m_username;
    m_usernameLabel->setText(uname.isEmpty() ? QStringLiteral("@username")
                                            : (uname.startsWith('@') ? uname : '@' + uname));
}

void MenuDrawer::refreshThemeIcon() {
    if (!m_editThemeButton || !m_themeManager) {
        return;
    }
    const QColor iconColor = m_themeManager->currentPalette().inputText;
    m_editThemeButton->setIcon(Lucide::icon(Lucide::Icon::Palette, iconColor, 18));
}
