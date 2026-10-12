#pragma once

#include <QColor>
#include <QLineEdit>
#include <QPointer>

class QVariantAnimation;
class ThemeManager;

class InputField final : public QLineEdit {
    Q_OBJECT

    Q_PROPERTY(QColor normalBorderColor READ normalBorderColor WRITE setNormalBorderColor)
    Q_PROPERTY(QColor focusedBorderColor READ focusedBorderColor WRITE setFocusedBorderColor)
    Q_PROPERTY(QColor disabledBorderColor READ disabledBorderColor WRITE setDisabledBorderColor)
    Q_PROPERTY(QColor placeholderColor READ placeholderColor WRITE setPlaceholderColor)
    Q_PROPERTY(int underlineHeight READ underlineHeight WRITE setUnderlineHeight)
    Q_PROPERTY(int focusedUnderlineHeight READ focusedUnderlineHeight WRITE setFocusedUnderlineHeight)
    Q_PROPERTY(int animationDuration READ animationDuration WRITE setAnimationDuration)

public:
    explicit InputField(QWidget* parent = nullptr, ThemeManager* themeManager = nullptr);

    void setThemeManager(ThemeManager* themeManager);

    [[nodiscard]] QColor normalBorderColor() const noexcept;
    void setNormalBorderColor(const QColor& color);

    [[nodiscard]] QColor focusedBorderColor() const noexcept;
    void setFocusedBorderColor(const QColor& color);

    [[nodiscard]] QColor disabledBorderColor() const noexcept;
    void setDisabledBorderColor(const QColor& color);

    [[nodiscard]] QColor placeholderColor() const noexcept;
    void setPlaceholderColor(const QColor& color);

    [[nodiscard]] int underlineHeight() const noexcept;
    void setUnderlineHeight(int height);

    [[nodiscard]] int focusedUnderlineHeight() const noexcept;
    void setFocusedUnderlineHeight(int height);

    [[nodiscard]] int animationDuration() const noexcept;
    void setAnimationDuration(int ms);

    void applyPalette(
        const QColor& normal,
        const QColor& focused,
        const QColor& disabled,
        const QColor& placeholder);

protected:
    void paintEvent(QPaintEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void startFocusAnimation(bool focused);
    void refreshCurrentColor();
    [[nodiscard]] QColor targetBorderColor() const noexcept;
    static QColor blendColors(const QColor& from, const QColor& to, double t) noexcept;

    QColor m_normalBorderColor = QColor("#777777");
    QColor m_focusedBorderColor = QColor("#000000");
    QColor m_disabledBorderColor = QColor("#BBBBBB");
    QColor m_placeholderColor = QColor("#767676");
    QColor m_currentBorderColor = QColor("#777777");

    int m_underlineHeight = 1;
    int m_focusedUnderlineHeight = 2;
    int m_animationDuration = 150;

    double m_focusProgress = 0.0;
    QPointer<QVariantAnimation> m_animation;
    ThemeManager* m_themeManager = nullptr;
};
