#pragma once

#include <QAbstractButton>
#include <QDialog>
#include <QString>
#include <QStringList>
#include <QVector>

#include "chat/chat_models.hpp"

class QLabel;
class QVBoxLayout;
class Button;
class InputField;
class ThemeManager;

class ContactRow final : public QAbstractButton {
    Q_OBJECT

public:
    explicit ContactRow(
        ThemeManager* themeManager,
        const chat::Contact& contact,
        QWidget* parent = nullptr);

    [[nodiscard]] QString contactId() const;
    [[nodiscard]] QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void refresh();

private:
    ThemeManager* m_themeManager = nullptr;
    chat::Contact m_contact;
};

class CreateGroupDialog final : public QDialog {
    Q_OBJECT

public:
    explicit CreateGroupDialog(
        ThemeManager* themeManager,
        const QVector<chat::Contact>& contacts,
        QWidget* parent = nullptr);

    [[nodiscard]] QString groupName() const;
    [[nodiscard]] QStringList selectedContactIds() const;

private slots:
    void tryAccept();
    void updateSelectionCount();
    void refreshTheme();

private:
    ThemeManager* m_themeManager = nullptr;

    InputField* m_nameInput = nullptr;
    QLabel* m_errorLabel = nullptr;
    QLabel* m_membersLabel = nullptr;
    QVector<ContactRow*> m_rows;
    Button* m_createButton = nullptr;
};
