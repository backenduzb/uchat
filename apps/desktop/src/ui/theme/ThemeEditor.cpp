#include "ThemeEditor.hpp"

#include <QAction>
#include <QButtonGroup>
#include <QColorDialog>
#include <QFont>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPaintEvent>
#include <QScrollArea>
#include <QVBoxLayout>

#include "animation/AnimationManager.hpp"
#include "components/IconButton.hpp"
#include "components/Lucide.hpp"
#include "theme/ThemeManager.hpp"

namespace {

constexpr int kEditorWidth = 400;
constexpr int kEditorMinWidth = 300;
constexpr int kEditorMargin = 20;
constexpr int kCardPreviewHeight = 96;
constexpr int kCardLabelHeight = 26;
constexpr int kSwatchSize = 38;
constexpr int kSwatchDot = 26;
constexpr int kAccentColumns = 5;

QString modeLabel(ThemeManager::Theme mode) {
    return mode == ThemeManager::Theme::Dark ? QStringLiteral("Dark")
                                            : QStringLiteral("Light");
}

void drawCheck(QPainter& painter, const QRectF& box, const QColor& color, qreal width) {
    painter.save();
    painter.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    const qreal left = box.left() + box.width() * 0.22;
    const qreal midX = box.left() + box.width() * 0.45;
    const qreal right = box.left() + box.width() * 0.80;
    const qreal midY = box.top() + box.height() * 0.55;
    painter.drawLine(QPointF(left, midY), QPointF(midX, box.top() + box.height() * 0.72));
    painter.drawLine(QPointF(midX, box.top() + box.height() * 0.72),
                     QPointF(right, box.top() + box.height() * 0.28));
    painter.restore();
}

}

ThemePreviewCard::ThemePreviewCard(
    ThemeManager* themeManager,
    ThemeManager::Theme mode,
    QWidget* parent)
    : QAbstractButton(parent), m_themeManager(themeManager), m_mode(mode) {
    setCheckable(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    setAccessibleName(modeLabel(mode));
    setToolTip(modeLabel(mode) + QStringLiteral(" mode"));
    if (m_themeManager) {
        connect(m_themeManager, &ThemeManager::appearanceChanged, this,
                [this]() { refresh(); });
    }
    refresh();
}

ThemeManager::Theme ThemePreviewCard::mode() const noexcept {
    return m_mode;
}

QSize ThemePreviewCard::sizeHint() const {
    return {170, kCardPreviewHeight + kCardLabelHeight + 20};
}

void ThemePreviewCard::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    const ThemeManager::SurfaceColors surfaces = ThemeManager::surfacesFor(m_mode);
    QColor accent("#229ED9");
    if (m_themeManager) {
        accent = m_themeManager->accentColor();
    }
    const QColor onAccent = ThemeManager::contrastText(accent);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF body = QRectF(rect()).adjusted(1.0, 1.0, -1.0, -1.0);
    const qreal radius = 12.0;

    painter.setPen(Qt::NoPen);
    painter.setBrush(surfaces.card);
    painter.drawRoundedRect(body, radius, radius);

    const qreal pad = 9.0;
    const qreal previewH = static_cast<qreal>(kCardPreviewHeight);
    const QRectF preview(body.left() + pad, body.top() + pad,
                         body.width() - pad * 2.0, previewH);
    painter.setBrush(surfaces.background);
    painter.drawRoundedRect(preview, 8.0, 8.0);

    const qreal sidebarW = preview.width() * 0.34;
    const QRectF sidebar(preview.left() + 5.0, preview.top() + 5.0,
                         sidebarW, preview.height() - 10.0);
    painter.setBrush(surfaces.surface);
    painter.drawRoundedRect(sidebar, 6.0, 6.0);

    for (int i = 0; i < 3; ++i) {
        const qreal rowY = sidebar.top() + 8.0 + static_cast<qreal>(i) * 22.0;
        if (rowY + 14.0 > sidebar.bottom() - 4.0) {
            break;
        }
        painter.setBrush(i == 0 ? accent : surfaces.muted);
        painter.drawEllipse(QPointF(sidebar.left() + 9.0, rowY + 5.0), 5.0, 5.0);
        painter.setBrush(i == 0 ? accent : surfaces.stroke);
        const QRectF line(sidebar.left() + 18.0, rowY + 2.0,
                          sidebar.width() - 24.0, 6.0);
        painter.drawRoundedRect(line, 3.0, 3.0);
    }

    const qreal chatX = sidebar.right() + 5.0;
    const qreal chatW = preview.right() - 5.0 - chatX;
    if (chatW > 20.0) {
        const QRectF incoming(chatX, preview.top() + 10.0, chatW * 0.72, 12.0);
        painter.setBrush(surfaces.field);
        painter.drawRoundedRect(incoming, 6.0, 6.0);
        const QRectF outgoing(chatX + chatW * 0.28, preview.top() + 28.0,
                              chatW * 0.72, 12.0);
        painter.setBrush(accent);
        painter.drawRoundedRect(outgoing, 6.0, 6.0);
        const QRectF incoming2(chatX, preview.top() + 46.0, chatW * 0.55, 12.0);
        painter.setBrush(surfaces.field);
        painter.drawRoundedRect(incoming2, 6.0, 6.0);
        Q_UNUSED(onAccent);
    }

    if (isChecked()) {
        const QPointF badgeCenter(preview.right() - 8.0, preview.top() + 8.0);
        painter.setPen(Qt::NoPen);
        painter.setBrush(accent);
        painter.drawEllipse(badgeCenter, 9.0, 9.0);
        drawCheck(painter, QRectF(badgeCenter.x() - 9.0, badgeCenter.y() - 9.0,
                                  18.0, 18.0),
                  onAccent, 2.0);
    }

    painter.setPen(surfaces.foreground);
    QFont labelFont = font();
    labelFont.setBold(true);
    painter.setFont(labelFont);
    const QRectF labelRect(body.left(), preview.bottom() + 2.0,
                           body.width(), static_cast<qreal>(kCardLabelHeight));
    painter.drawText(labelRect, Qt::AlignCenter, modeLabel(m_mode));

    QColor border = surfaces.stroke;
    qreal borderWidth = 1.0;
    if (isChecked()) {
        border = accent;
        borderWidth = 2.0;
    } else if (hasFocus()) {
        border = accent;
        borderWidth = 1.5;
    }
    painter.setPen(QPen(border, borderWidth));
    painter.setBrush(Qt::NoBrush);
    painter.drawRoundedRect(body, radius, radius);
}

void ThemePreviewCard::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        click();
        event->accept();
        return;
    }
    QAbstractButton::keyPressEvent(event);
}

void ThemePreviewCard::refresh() {
    updateCardStyle();
    update();
}

void ThemePreviewCard::updateCardStyle() {
    setStyleSheet(QStringLiteral("ThemePreviewCard { background: transparent; border: none; }"));
}

AccentSwatch::AccentSwatch(
    const QColor& color,
    const QString& name,
    bool isCustomSlot,
    QWidget* parent)
    : QAbstractButton(parent), m_color(color), m_isCustomSlot(isCustomSlot) {
    setCheckable(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    setFixedSize(kSwatchSize, kSwatchSize);
    setAccessibleName(name);
    setToolTip(name);
    setStyleSheet(QStringLiteral("AccentSwatch { background: transparent; border: none; }"));
}

void AccentSwatch::setSwatchColor(const QColor& color) {
    if (m_color == color) {
        return;
    }
    m_color = color;
    update();
}

QColor AccentSwatch::swatchColor() const noexcept {
    return m_color;
}

QSize AccentSwatch::sizeHint() const {
    return {kSwatchSize, kSwatchSize};
}

void AccentSwatch::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QPointF center(width() / 2.0, height() / 2.0);
    const qreal dotR = static_cast<qreal>(kSwatchDot) / 2.0;

    if (m_isCustomSlot && !m_color.isValid()) {
        painter.setPen(QPen(QColor("#8E8E93"), 1.5, Qt::DashLine, Qt::RoundCap));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(center, dotR, dotR);
        const QPixmap plus = Lucide::pixmap(Lucide::Icon::Plus, 14, QColor("#8E8E93"),
                                            2.0, painter.device()->devicePixelRatioF());
        painter.drawPixmap(QPointF(center.x() - plus.width() / 2.0 / plus.devicePixelRatio(),
                                   center.y() - plus.height() / 2.0 / plus.devicePixelRatio()),
                           plus);
    } else {
        painter.setPen(Qt::NoPen);
        painter.setBrush(m_color);
        painter.drawEllipse(center, dotR, dotR);
        if (isChecked()) {
            drawCheck(painter, QRectF(center.x() - dotR, center.y() - dotR,
                                      dotR * 2.0, dotR * 2.0),
                      ThemeManager::contrastText(m_color), 2.2);
        }
    }

    if (isChecked() && m_color.isValid()) {
        painter.setPen(QPen(m_color, 2.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(center, dotR + 4.0, dotR + 4.0);
    } else if (hasFocus()) {
        painter.setPen(QPen(QColor("#8E8E93"), 1.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(center, dotR + 4.0, dotR + 4.0);
    }
}

void AccentSwatch::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        click();
        event->accept();
        return;
    }
    QAbstractButton::keyPressEvent(event);
}

ThemeEditor::ThemeEditor(ThemeManager* themeManager, QWidget* parent)
    : QWidget(parent), m_themeManager(themeManager) {
    setObjectName("themeEditor");
    setAttribute(Qt::WA_StyledBackground, true);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    setMinimumWidth(kEditorMinWidth);
    setMaximumWidth(kEditorWidth);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(kEditorMargin, 16, kEditorMargin, kEditorMargin);
    root->setSpacing(10);

    auto* header = new QHBoxLayout();
    header->setContentsMargins(0, 0, 0, 0);
    header->setSpacing(12);

    auto* titleBox = new QVBoxLayout();
    titleBox->setContentsMargins(0, 0, 0, 0);
    titleBox->setSpacing(2);
    m_titleLabel = new QLabel(QStringLiteral("Appearance"), this);
    m_titleLabel->setObjectName("editorTitle");
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_subtitleLabel = new QLabel(QStringLiteral("Customize the look and feel."), this);
    m_subtitleLabel->setObjectName("editorSubtitle");
    m_subtitleLabel->setWordWrap(true);
    titleBox->addWidget(m_titleLabel);
    titleBox->addWidget(m_subtitleLabel);
    header->addLayout(titleBox, 1);

    m_closeButton = new IconButton(Lucide::Icon::X, this, m_themeManager);
    m_closeButton->setObjectName("editorCloseButton");
    m_closeButton->setFixedSize(32, 32);
    m_closeButton->setIconSize(16);
    m_closeButton->setToolTip(QStringLiteral("Close"));
    m_closeButton->setAccessibleName(QStringLiteral("Close theme editor"));
    connect(m_closeButton, &QAbstractButton::clicked, this, &ThemeEditor::closeRequested);
    header->addWidget(m_closeButton, 0, Qt::AlignTop);
    root->addLayout(header);

    m_separator = new QWidget(this);
    m_separator->setObjectName("editorSeparator");
    m_separator->setFixedHeight(1);
    m_separator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    root->addWidget(m_separator);

    auto* scroll = new QScrollArea(this);
    scroll->setObjectName("editorScroll");
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setAttribute(Qt::WA_StyledBackground, true);

    auto* content = new QWidget(scroll);
    content->setObjectName("editorContent");
    content->setAttribute(Qt::WA_StyledBackground, true);
    auto* contentLayout = new QVBoxLayout(content);
    contentLayout->setContentsMargins(0, 2, 0, 2);
    contentLayout->setSpacing(10);

    m_modeSectionLabel = new QLabel(QStringLiteral("Color mode"), content);
    m_modeSectionLabel->setObjectName("editorSection");
    QFont sectionFont = m_modeSectionLabel->font();
    sectionFont.setBold(true);
    m_modeSectionLabel->setFont(sectionFont);

    auto* cardsRow = new QHBoxLayout();
    cardsRow->setContentsMargins(0, 0, 0, 0);
    cardsRow->setSpacing(12);
    m_darkCard = new ThemePreviewCard(m_themeManager, ThemeManager::Theme::Dark, content);
    m_lightCard = new ThemePreviewCard(m_themeManager, ThemeManager::Theme::Light, content);
    m_modeGroup = new QButtonGroup(this);
    m_modeGroup->setExclusive(true);
    m_modeGroup->addButton(m_darkCard, 0);
    m_modeGroup->addButton(m_lightCard, 1);
    cardsRow->addWidget(m_darkCard, 1);
    cardsRow->addWidget(m_lightCard, 1);
    connect(m_modeGroup, &QButtonGroup::buttonClicked, this,
            &ThemeEditor::applySelectedMode);

    m_accentSectionLabel = new QLabel(QStringLiteral("Accent color"), content);
    m_accentSectionLabel->setObjectName("editorSection");
    m_accentSectionLabel->setFont(sectionFont);

    auto* accentBox = new QWidget(content);
    accentBox->setObjectName("editorAccentBox");
    m_accentGrid = new QGridLayout(accentBox);
    m_accentGrid->setContentsMargins(0, 0, 0, 0);
    m_accentGrid->setHorizontalSpacing(8);
    m_accentGrid->setVerticalSpacing(8);

    m_accentGroup = new QButtonGroup(this);
    m_accentGroup->setExclusive(true);
    const QVector<ThemeManager::AccentOption> options = ThemeManager::defaultAccents();
    const int optionCount = static_cast<int>(options.size());
    for (int i = 0; i < optionCount; ++i) {
        const auto& option = options.at(i);
        auto* swatch = new AccentSwatch(option.color, option.name, false, accentBox);
        swatch->setProperty("accentName", option.color.name());
        m_swatches.push_back(swatch);
        m_accentGroup->addButton(swatch, i);
        m_accentGrid->addWidget(swatch, i / kAccentColumns, i % kAccentColumns);
    }
    m_customSwatch = new AccentSwatch(QColor(), QStringLiteral("Custom color"), true,
                                      accentBox);
    m_accentGroup->addButton(m_customSwatch, optionCount);
    m_accentGrid->addWidget(m_customSwatch, optionCount / kAccentColumns,
                            optionCount % kAccentColumns);
    auto* gridStretch = new QWidget(accentBox);
    gridStretch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_accentGrid->addWidget(gridStretch, 0, kAccentColumns);
    connect(m_accentGroup, &QButtonGroup::buttonClicked, this,
            &ThemeEditor::applySelectedAccent);

    m_hintLabel = new QLabel(
        QStringLiteral("Changes apply instantly and are saved automatically."), content);
    m_hintLabel->setObjectName("editorHint");
    m_hintLabel->setWordWrap(true);

    contentLayout->addWidget(m_modeSectionLabel);
    contentLayout->addLayout(cardsRow);
    contentLayout->addSpacing(4);
    contentLayout->addWidget(m_accentSectionLabel);
    contentLayout->addWidget(accentBox);
    contentLayout->addSpacing(2);
    contentLayout->addWidget(m_hintLabel);
    contentLayout->addStretch(1);

    scroll->setWidget(content);
    root->addWidget(scroll, 1);

    if (m_themeManager) {
        connect(m_themeManager, &ThemeManager::appearanceChanged, this,
                &ThemeEditor::refresh);
    }
    refresh();
}

void ThemeEditor::refresh() {
    refreshTheme();
    syncSelection();
}

void ThemeEditor::applySelectedMode() {
    if (!m_themeManager) {
        return;
    }
    const QAbstractButton* checked = m_modeGroup ? m_modeGroup->checkedButton() : nullptr;
    if (checked == m_darkCard) {
        m_themeManager->setTheme(ThemeManager::Theme::Dark);
    } else if (checked == m_lightCard) {
        m_themeManager->setTheme(ThemeManager::Theme::Light);
    }
    syncSelection();
}

void ThemeEditor::applySelectedAccent() {
    if (!m_themeManager || !m_accentGroup) {
        return;
    }
    QAbstractButton* checked = m_accentGroup->checkedButton();
    if (checked == nullptr) {
        return;
    }
    auto* swatch = qobject_cast<AccentSwatch*>(checked);
    if (swatch == nullptr) {
        return;
    }
    if (swatch == m_customSwatch) {
        chooseCustomColor();
        return;
    }
    if (swatch->swatchColor().isValid()) {
        m_themeManager->setAccentColor(swatch->swatchColor());
    }
    syncSelection();
}

void ThemeEditor::chooseCustomColor() {
    if (!m_themeManager) {
        return;
    }
    const QColor start = m_themeManager->customAccent().isValid()
        ? m_themeManager->customAccent()
        : m_themeManager->accentColor();
    const QColor chosen = QColorDialog::getColor(start, this,
                                                QStringLiteral("Custom accent color"));
    if (!chosen.isValid() || !ThemeManager::isValidAccent(chosen)) {
        syncSelection();
        return;
    }
    m_themeManager->setCustomAccent(chosen);
    syncSelection();
}

void ThemeEditor::refreshTheme() {
    if (!m_themeManager) {
        return;
    }
    const ThemeManager::SurfaceColors surfaces = m_themeManager->surfaceColors();
    const QColor accent = m_themeManager->accentColor();

    setStyleSheet(QStringLiteral(
        "ThemeEditor#themeEditor { background-color: %1; border: 1px solid %2; "
        "border-radius: 16px; }"
        "QLabel#editorTitle { background: transparent; border: none; color: %3; }"
        "QLabel#editorSubtitle { background: transparent; border: none; color: %4; }"
        "QLabel#editorSection { background: transparent; border: none; color: %3; }"
        "QLabel#editorHint { background: transparent; border: none; color: %4; }"
        "QWidget#editorSeparator { background-color: %2; border: none; }"
        "QScrollArea#editorScroll { background: transparent; border: none; }"
        "QWidget#editorContent { background: transparent; border: none; }"
        "QWidget#editorAccentBox { background: transparent; border: none; }"
        "QScrollBar:vertical { background: transparent; width: 8px; margin: 2px; }"
        "QScrollBar::handle:vertical { background: %2; border-radius: 4px; min-height: 24px; }"
        "QScrollBar::handle:vertical:hover { background: %5; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }")
        .arg(surfaces.card.name(), surfaces.stroke.name(), surfaces.foreground.name(),
             surfaces.muted.name(), accent.name()));

    updateCustomSlot();
    if (m_darkCard) {
        m_darkCard->update();
    }
    if (m_lightCard) {
        m_lightCard->update();
    }
    for (AccentSwatch* swatch : m_swatches) {
        if (swatch) {
            swatch->update();
        }
    }
    if (m_customSwatch) {
        m_customSwatch->update();
    }
}

void ThemeEditor::syncSelection() {
    if (!m_themeManager) {
        return;
    }
    const bool isDark =
        (m_themeManager->currentTheme() == ThemeManager::Theme::Dark);
    if (m_modeGroup) {
        m_modeGroup->blockSignals(true);
        m_darkCard->setChecked(isDark);
        m_lightCard->setChecked(!isDark);
        m_modeGroup->blockSignals(false);
    }

    updateCustomSlot();
    if (m_accentGroup) {
        m_accentGroup->blockSignals(true);
        const QColor accent = m_themeManager->accentColor();
        const QColor custom = m_themeManager->customAccent();
        bool matched = false;
        for (AccentSwatch* swatch : m_swatches) {
            const bool isCurrent =
                swatch && swatch->swatchColor().isValid() && swatch->swatchColor() == accent;
            swatch->setChecked(isCurrent);
            matched = matched || isCurrent;
        }
        const bool customCurrent =
            custom.isValid() && accent == custom && m_customSwatch != nullptr;
        if (m_customSwatch) {
            m_customSwatch->setChecked(customCurrent);
        }
        matched = matched || customCurrent;
        if (!matched && m_customSwatch && custom.isValid()) {
            m_customSwatch->setChecked(false);
        }
        m_accentGroup->blockSignals(false);
    }
    update();
}

void ThemeEditor::updateCustomSlot() {
    if (!m_customSwatch || !m_themeManager) {
        return;
    }
    const QColor custom = m_themeManager->customAccent();
    if (custom.isValid()) {
        m_customSwatch->setSwatchColor(custom);
        m_customSwatch->setToolTip(
            QStringLiteral("Custom color (%1). Click to edit.").arg(custom.name()));
        m_customSwatch->setAccessibleName(
            QStringLiteral("Custom accent color %1").arg(custom.name()));
    } else {
        m_customSwatch->setSwatchColor(QColor());
        m_customSwatch->setToolTip(QStringLiteral("Choose a custom color"));
        m_customSwatch->setAccessibleName(QStringLiteral("Choose a custom color"));
    }
}

ThemeEditorOverlay::ThemeEditorOverlay(
    ThemeManager* themeManager,
    AnimationManager* animationManager,
    QWidget* parent)
    : QWidget(parent), m_themeManager(themeManager), m_animationManager(animationManager) {
    setObjectName("themeEditorOverlay");
    setAttribute(Qt::WA_StyledBackground, true);
    setVisible(false);

    auto* escAction = new QAction(this);
    escAction->setShortcut(Qt::Key_Escape);
    escAction->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    connect(escAction, &QAction::triggered, this, &ThemeEditorOverlay::closeEditor);
    addAction(escAction);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    m_editor = new ThemeEditor(m_themeManager, this);
    layout->addWidget(m_editor, 0, Qt::AlignCenter);

    connect(m_editor, &ThemeEditor::closeRequested, this,
            &ThemeEditorOverlay::closeEditor);

    if (parent) {
        parent->installEventFilter(this);
        fitToParent();
    }
}

bool ThemeEditorOverlay::isEditorOpen() const noexcept {
    return m_open;
}

void ThemeEditorOverlay::openEditor() {
    if (m_open || !parentWidget()) {
        return;
    }
    fitToParent();
    m_open = true;
    m_editor->refresh();
    setVisible(true);
    raise();
    constrainEditor();
    if (m_animationManager) {
        m_animationManager->fadeIn(m_editor, 160);
    }
    m_editor->setFocus(Qt::PopupFocusReason);
}

void ThemeEditorOverlay::closeEditor() {
    if (!m_open) {
        return;
    }
    m_open = false;
    setVisible(false);
    emit editorClosed();
}

bool ThemeEditorOverlay::eventFilter(QObject* watched, QEvent* event) {
    if (watched == parentWidget() && event->type() == QEvent::Resize) {
        fitToParent();
    }
    return QWidget::eventFilter(watched, event);
}

void ThemeEditorOverlay::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 140));
    painter.drawRect(rect());
}

void ThemeEditorOverlay::mousePressEvent(QMouseEvent* event) {
    event->accept();
    closeEditor();
}

void ThemeEditorOverlay::fitToParent() {
    QWidget* container = parentWidget();
    if (!container) {
        return;
    }
    setGeometry(container->rect());
    constrainEditor();
}

void ThemeEditorOverlay::constrainEditor() {
    if (!m_editor || !parentWidget()) {
        return;
    }
    const int available = parentWidget()->height() - 48;
    m_editor->setMaximumHeight(available < 320 ? 320 : available);
}
