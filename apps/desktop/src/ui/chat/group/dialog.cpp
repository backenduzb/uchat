#include "ui/chat/group/dialog.hpp"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include "components/Button.hpp"
#include "components/InputField.hpp"
#include "theme/ThemeManager.hpp"

namespace {

constexpr int kDialogWidth = 400;
constexpr int kRowHeight = 56;
constexpr int kRowAvatarSize = 36;

void drawRowCheck(QPainter& painter, const QPointF& center, qreal radius,
                  const QColor& accent, const QColor& onAccent, bool checked,
                  const QColor& stroke) {
    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);
    if (checked) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(accent);
        painter.drawEllipse(center, radius, radius);
        painter.setPen(QPen(onAccent, 2.0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        const qreal left = center.x() - radius * 0.45;
        const qreal midX = center.x() - radius * 0.05;
        const qreal right = center.x() + radius * 0.5;
        const qreal midY = center.y() + radius * 0.05;
        painter.drawLine(QPointF(left, midY), QPointF(midX, center.y() + radius * 0.35));
        painter.drawLine(QPointF(midX, center.y() + radius * 0.35),
                         QPointF(right, center.y() - radius * 0.4));
    } else {
        painter.setPen(QPen(stroke, 1.5));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(center, radius, radius);
    }
    painter.restore();
}

} // namespace

ContactRow::ContactRow(
    ThemeManager* themeManager,
    const chat::Contact& contact,
    QWidget* parent)
    : QAbstractButton(parent), m_themeManager(themeManager), m_contact(contact) {
    setCheckable(true);
    setFocusPolicy(Qt::StrongFocus);
    setCursor(Qt::PointingHandCursor);
    setFixedHeight(kRowHeight);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    setAccessibleName(contact.displayName);
    setToolTip(contact.displayName);
    if (m_themeManager) {
        connect(m_themeManager, &ThemeManager::appearanceChanged, this,
                &ContactRow::refresh);
    }
}

QString ContactRow::contactId() const {
    return m_contact.id;
}

QSize ContactRow::sizeHint() const {
    return {200, kRowHeight};
}

void ContactRow::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QColor foreground("#000000");
    QColor muted("#767676");
    QColor accent("#229ED9");
    QColor hoverBg;
    bool hasHoverBg = false;
    if (m_themeManager) {
        const ThemeManager::SurfaceColors surfaces = m_themeManager->surfaceColors();
        foreground = surfaces.foreground;
        muted = surfaces.muted;
        accent = m_themeManager->accentColor();
        if (underMouse()) {
            hoverBg = surfaces.surface;
            hasHoverBg = true;
        }
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (isChecked()) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(accent.red(), accent.green(), accent.blue(), 28));
        painter.drawRoundedRect(QRectF(rect()).adjusted(1.0, 1.0, -1.0, -1.0), 10.0, 10.0);
    } else if (hasHoverBg) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(hoverBg);
        painter.drawRoundedRect(QRectF(rect()).adjusted(1.0, 1.0, -1.0, -1.0), 10.0, 10.0);
    }

    const qreal avatarR = static_cast<qreal>(kRowAvatarSize) / 2.0;
    const QPointF avatarCenter(12.0 + avatarR, height() / 2.0);
    const QColor avatarColor = chat::avatarColorFor(m_contact.id);
    painter.setPen(Qt::NoPen);
    painter.setBrush(avatarColor);
    painter.drawEllipse(avatarCenter, avatarR, avatarR);

    painter.setPen(chat::avatarTextColorFor(avatarColor));
    QFont initialFont = font();
    initialFont.setBold(true);
    painter.setFont(initialFont);
    painter.drawText(QRectF(avatarCenter.x() - avatarR, avatarCenter.y() - avatarR,
                            avatarR * 2.0, avatarR * 2.0),
                     Qt::AlignCenter, chat::contactInitials(m_contact.displayName));

    painter.setPen(foreground);
    QFont nameFont = font();
    nameFont.setBold(isChecked());
    painter.setFont(nameFont);
    const qreal textX = 12.0 + static_cast<qreal>(kRowAvatarSize) + 10.0;
    painter.drawText(QRectF(textX, 0, width() - textX - 44.0, height()),
                     Qt::AlignVCenter | Qt::AlignLeft, m_contact.displayName);

    const QPointF checkCenter(width() - 24.0, height() / 2.0);
    drawRowCheck(painter, checkCenter, 10.0, accent,
                 ThemeManager::contrastText(accent), isChecked(), muted);
}

void ContactRow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return) {
        click();
        event->accept();
        return;
    }
    QAbstractButton::keyPressEvent(event);
}

void ContactRow::refresh() {
    update();
}

CreateGroupDialog::CreateGroupDialog(
    ThemeManager* themeManager,
    const QVector<chat::Contact>& contacts,
    QWidget* parent)
    : QDialog(parent), m_themeManager(themeManager) {
    setObjectName("createGroupDialog");
    setWindowTitle(QStringLiteral("Create Group"));
    setModal(true);
    setFixedWidth(kDialogWidth);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(20, 18, 20, 18);
    root->setSpacing(10);

    auto* title = new QLabel(QStringLiteral("Create Group"), this);
    title->setObjectName("dialogTitle");
    QFont titleFont = title->font();
    titleFont.setPointSize(14);
    titleFont.setBold(true);
    title->setFont(titleFont);
    root->addWidget(title);

    auto* subtitle = new QLabel(
        QStringLiteral("Choose a name and add members to start a new group chat."), this);
    subtitle->setObjectName("dialogSubtitle");
    subtitle->setWordWrap(true);
    root->addWidget(subtitle);

    m_nameInput = new InputField(this, m_themeManager);
    m_nameInput->setObjectName("groupNameInput");
    m_nameInput->setPlaceholderText(QStringLiteral("Group name"));
    m_nameInput->setMaxLength(64);
    root->addWidget(m_nameInput);

    m_errorLabel = new QLabel(this);
    m_errorLabel->setObjectName("errorLabel");
    m_errorLabel->setWordWrap(true);
    m_errorLabel->setVisible(false);
    root->addWidget(m_errorLabel);

    m_membersLabel = new QLabel(this);
    m_membersLabel->setObjectName("dialogSection");
    QFont sectionFont = m_membersLabel->font();
    sectionFont.setBold(true);
    m_membersLabel->setFont(sectionFont);
    root->addWidget(m_membersLabel);

    auto* scroll = new QScrollArea(this);
    scroll->setObjectName("dialogScroll");
    scroll->setWidgetResizable(true);
    scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setMinimumHeight(180);
    scroll->setMaximumHeight(280);

    auto* listContent = new QWidget(scroll);
    listContent->setObjectName("dialogListContent");
    auto* listLayout = new QVBoxLayout(listContent);
    listLayout->setContentsMargins(0, 0, 0, 0);
    listLayout->setSpacing(2);
    for (const auto& contact : contacts) {
        auto* row = new ContactRow(m_themeManager, contact, listContent);
        connect(row, &QAbstractButton::toggled, this,
                &CreateGroupDialog::updateSelectionCount);
        m_rows.push_back(row);
        listLayout->addWidget(row);
    }
    listLayout->addStretch(1);
    scroll->setWidget(listContent);
    root->addWidget(scroll, 1);

    auto* buttons = new QHBoxLayout();
    buttons->setContentsMargins(0, 0, 0, 0);
    buttons->setSpacing(8);
    buttons->addStretch(1);

    auto* cancelButton = new QPushButton(QStringLiteral("Cancel"), this);
    cancelButton->setObjectName("dialogCancel");
    cancelButton->setCursor(Qt::PointingHandCursor);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttons->addWidget(cancelButton);

    m_createButton = new Button(QStringLiteral("Create Group"), this, m_themeManager);
    m_createButton->setObjectName("dialogCreate");
    m_createButton->setCursor(Qt::PointingHandCursor);
    connect(m_createButton, &QPushButton::clicked, this, &CreateGroupDialog::tryAccept);
    buttons->addWidget(m_createButton);
    root->addLayout(buttons);

    if (m_themeManager) {
        connect(m_themeManager, &ThemeManager::appearanceChanged, this,
                &CreateGroupDialog::refreshTheme);
    }
    updateSelectionCount();
    refreshTheme();
    m_nameInput->setFocus(Qt::OtherFocusReason);
}

QString CreateGroupDialog::groupName() const {
    return m_nameInput ? m_nameInput->text().trimmed() : QString();
}

QStringList CreateGroupDialog::selectedContactIds() const {
    QStringList ids;
    for (const ContactRow* row : m_rows) {
        if (row && row->isChecked()) {
            ids.push_back(row->contactId());
        }
    }
    return ids;
}

void CreateGroupDialog::tryAccept() {
    if (groupName().isEmpty()) {
        m_errorLabel->setText(QStringLiteral("Please enter a group name."));
        m_errorLabel->setVisible(true);
        m_nameInput->setFocus(Qt::OtherFocusReason);
        return;
    }
    if (selectedContactIds().isEmpty()) {
        m_errorLabel->setText(QStringLiteral("Select at least one participant."));
        m_errorLabel->setVisible(true);
        return;
    }
    m_errorLabel->setVisible(false);
    accept();
}

void CreateGroupDialog::updateSelectionCount() {
    const int count = static_cast<int>(selectedContactIds().size());
    m_membersLabel->setText(tr("Members · %n selected", "", count));
    for (ContactRow* row : m_rows) {
        if (row) {
            row->update();
        }
    }
}

void CreateGroupDialog::refreshTheme() {
    if (!m_themeManager) {
        return;
    }
    const ThemeManager::SurfaceColors surfaces = m_themeManager->surfaceColors();
    setStyleSheet(QStringLiteral(
        "CreateGroupDialog#createGroupDialog { background-color: %1; border: 1px solid %2; }"
        "QLabel#dialogTitle { background: transparent; border: none; color: %3; }"
        "QLabel#dialogSubtitle { background: transparent; border: none; color: %4; }"
        "QLabel#dialogSection { background: transparent; border: none; color: %3; }"
        "QScrollArea#dialogScroll { background: transparent; border: none; }"
        "QWidget#dialogListContent { background: transparent; border: none; }"
        "QPushButton#dialogCancel { background-color: %5; color: %3; border: 1px solid %2; "
        "border-radius: 10px; padding: 10px 18px; font-weight: 600; }"
        "QPushButton#dialogCancel:hover { background-color: %6; }"
        "QScrollBar:vertical { background: transparent; width: 8px; margin: 2px; }"
        "QScrollBar::handle:vertical { background: %2; border-radius: 4px; min-height: 24px; }"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0px; }"
        "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical { background: none; }")
        .arg(surfaces.card.name(), surfaces.stroke.name(), surfaces.foreground.name(),
             surfaces.muted.name(), surfaces.surface.name(), surfaces.field.name()));
    updateSelectionCount();
}
