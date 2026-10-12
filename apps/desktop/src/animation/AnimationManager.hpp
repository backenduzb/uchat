#pragma once

#include <QObject>
#include <QPoint>
#include <QPointer>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QWidget>
#include <QEasingCurve>
#include <QColor>
#include <QVariantAnimation>
#include <functional>
#include <qpropertyanimation.h>

class AnimationManager final: public QObject {
    Q_OBJECT
    public:
    static constexpr int kDefaultColorDurationMs = 150;

    enum class SlideDirection {
        Forward,  
        Backward, 
    };

    explicit AnimationManager(QObject *parent = nullptr);
    void fadeIn(QWidget *widget, int duration = 350);
    void slideUp(QWidget *widget, int distance = 24, int duration = 400);
    void fadeAndSlideIn(QWidget *widget, int distance = 24, int duration = 450);
    void shake(QWidget *widget, int distance = 8, int duration = 300);

    void slideReplace(
        QWidget *widget,
        std::function<void()> swapContent,
        SlideDirection direction = SlideDirection::Forward,
        int distance = 48,
        int duration = 320,
        std::function<void()> onFinished = {});

    static QVariantAnimation *animateColorValue(
        const QColor &from,
        const QColor &to,
        int duration,
        QObject *context,
        std::function<void(const QColor &)> onUpdate,
        const QEasingCurve &easing = QEasingCurve(QEasingCurve::OutCubic));

    static QVariantAnimation *animateProgress(
        double from,
        double to,
        int duration,
        QObject *context,
        std::function<void(double)> onUpdate,
        const QEasingCurve &easing = QEasingCurve(QEasingCurve::OutCubic));

    static void stopAnimation(QPointer<QVariantAnimation> &animation);

    private:
        QPointer<QParallelAnimationGroup> m_entryAnimation;
        QPointer<QPropertyAnimation> m_shakeAnimation;

        QPointer<QAbstractAnimation> m_swapAnimation;
        QPointer<QWidget> m_swapWidget;
        QPoint m_swapOrigin;
        bool m_swapOwnedEffect = false;
};