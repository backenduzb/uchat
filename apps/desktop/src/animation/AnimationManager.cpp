#include "AnimationManager.hpp"

#include <QEasingCurve>
#include <QGraphicsOpacityEffect>
#include <QPauseAnimation>
#include <QPoint>
#include <algorithm>

AnimationManager::AnimationManager(QObject* parent)
    : QObject(parent) {}

void AnimationManager::fadeIn(QWidget* widget, int duration) {
    if (!widget) {
        return;
    }

    auto* effect = new QGraphicsOpacityEffect(widget);
    effect->setOpacity(0.0);
    widget->setGraphicsEffect(effect);

    auto* animation = new QPropertyAnimation(effect, "opacity", this);
    animation->setDuration(duration);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(animation, &QPropertyAnimation::finished,
            animation, &QObject::deleteLater);

    animation->start();
}

void AnimationManager::slideUp(
    QWidget* widget,
    int distance,
    int duration
) {
    if (!widget) {
        return;
    }

    const QPoint finalPosition = widget->pos();
    widget->move(finalPosition + QPoint(0, distance));

    auto* animation = new QPropertyAnimation(widget, "pos", this);
    animation->setDuration(duration);
    animation->setStartValue(widget->pos());
    animation->setEndValue(finalPosition);
    animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(animation, &QPropertyAnimation::finished,
            animation, &QObject::deleteLater);

    animation->start();
}

void AnimationManager::fadeAndSlideIn(
    QWidget* widget,
    int distance,
    int duration
) {
    if (!widget) {
        return;
    }

    const QPoint finalPosition = widget->pos();

    auto* effect = new QGraphicsOpacityEffect(widget);
    effect->setOpacity(0.0);
    widget->setGraphicsEffect(effect);

    widget->move(finalPosition + QPoint(0, distance));

    auto* group = new QParallelAnimationGroup(this);

    auto* fade = new QPropertyAnimation(effect, "opacity", group);
    fade->setDuration(duration);
    fade->setStartValue(0.0);
    fade->setEndValue(1.0);
    fade->setEasingCurve(QEasingCurve::OutCubic);

    auto* slide = new QPropertyAnimation(widget, "pos", group);
    slide->setDuration(duration);
    slide->setStartValue(widget->pos());
    slide->setEndValue(finalPosition);
    slide->setEasingCurve(QEasingCurve::OutCubic);

    connect(group, &QParallelAnimationGroup::finished,
            group, &QObject::deleteLater);

    group->start();
}

void AnimationManager::shake(
    QWidget* widget,
    int distance,
    int duration
) {
    if (!widget) {
        return;
    }

    const QPoint originalPosition = widget->pos();

    auto* animation = new QPropertyAnimation(widget, "pos", this);
    animation->setDuration(duration);
    animation->setEasingCurve(QEasingCurve::OutCubic);

    animation->setKeyValueAt(0.0, originalPosition);
    animation->setKeyValueAt(0.2, originalPosition + QPoint(distance, 0));
    animation->setKeyValueAt(0.4, originalPosition + QPoint(-distance, 0));
    animation->setKeyValueAt(0.6, originalPosition + QPoint(distance / 2, 0));
    animation->setKeyValueAt(0.8, originalPosition - QPoint(distance / 2, 0));
    animation->setKeyValueAt(1.0, originalPosition);

    connect(animation, &QPropertyAnimation::finished,
            animation, &QObject::deleteLater);

    animation->start();
}

QVariantAnimation* AnimationManager::animateColorValue(
    const QColor& from,
    const QColor& to,
    int duration,
    QObject* context,
    std::function<void(const QColor&)> onUpdate,
    const QEasingCurve& easing
) {
    auto* animation = new QVariantAnimation(context);
    animation->setDuration(duration);
    animation->setStartValue(from);
    animation->setEndValue(to);
    animation->setEasingCurve(easing);

    QObject::connect(animation, &QVariantAnimation::valueChanged, context,
                     [onUpdate](const QVariant& value) {
                         if (onUpdate) {
                             onUpdate(value.value<QColor>());
                         }
                     });

    QObject::connect(animation, &QVariantAnimation::finished,
                     animation, &QObject::deleteLater);

    animation->start();
    return animation;
}

QVariantAnimation* AnimationManager::animateProgress(
    double from,
    double to,
    int duration,
    QObject* context,
    std::function<void(double)> onUpdate,
    const QEasingCurve& easing
) {
    auto* animation = new QVariantAnimation(context);
    animation->setDuration(duration);
    animation->setStartValue(from);
    animation->setEndValue(to);
    animation->setEasingCurve(easing);

    QObject::connect(animation, &QVariantAnimation::valueChanged, context,
                     [onUpdate](const QVariant& value) {
                         if (onUpdate) {
                             onUpdate(value.toDouble());
                         }
                     });

    QObject::connect(animation, &QVariantAnimation::finished,
                     animation, &QObject::deleteLater);

    animation->start();
    return animation;
}

void AnimationManager::stopAnimation(QPointer<QVariantAnimation>& animation) {
    if (!animation.isNull()) {
        animation->stop();
        animation->deleteLater();
        animation.clear();
    }
}

void AnimationManager::slideReplace(
    QWidget* widget,
    std::function<void()> swapContent,
    SlideDirection direction,
    int distance,
    int duration,
    std::function<void()> onFinished
) {
    if (!widget) {
        return;
    }

    if (!m_swapAnimation.isNull()) {
        m_swapAnimation->stop();
        if (!m_swapWidget.isNull()) {
            m_swapWidget->move(m_swapOrigin);
            if (auto* fx = qobject_cast<QGraphicsOpacityEffect*>(
                    m_swapWidget->graphicsEffect())) {
                fx->setOpacity(1.0);
                if (m_swapOwnedEffect) {
                    m_swapWidget->setGraphicsEffect(nullptr);
                }
            }
        }
        m_swapAnimation.clear();
        m_swapWidget.clear();
        m_swapOwnedEffect = false;
    }

    const QPoint origin = widget->pos();
    const int sign = (direction == SlideDirection::Forward) ? 1 : -1;
    const int half = std::max(1, duration / 2);

    auto* effect = qobject_cast<QGraphicsOpacityEffect*>(widget->graphicsEffect());
    const bool owned = (effect == nullptr);
    if (!effect) {
        effect = new QGraphicsOpacityEffect(widget);
        widget->setGraphicsEffect(effect);
    }
    effect->setOpacity(1.0);

    auto* out = new QParallelAnimationGroup(this);

    auto* outPos = new QPropertyAnimation(widget, "pos", out);
    outPos->setDuration(half);
    outPos->setStartValue(origin);
    outPos->setEndValue(origin + QPoint(-sign * distance, 0));
    outPos->setEasingCurve(QEasingCurve::InCubic);

    auto* outFade = new QPropertyAnimation(effect, "opacity", out);
    outFade->setDuration(half);
    outFade->setStartValue(1.0);
    outFade->setEndValue(0.0);
    outFade->setEasingCurve(QEasingCurve::InCubic);

    m_swapAnimation = out;
    m_swapWidget = widget;
    m_swapOrigin = origin;
    m_swapOwnedEffect = owned;

    const QPointer<QWidget> weakWidget(widget);

    connect(out, &QParallelAnimationGroup::finished, this,
            [this, weakWidget, swapContent, onFinished, origin, sign, distance, half, owned]() {
                if (weakWidget.isNull()) {
                    m_swapAnimation.clear();
                    m_swapWidget.clear();
                    return;
                }

                QWidget* target = weakWidget.data();
                if (swapContent) {
                    swapContent();
                }

                target->move(origin + QPoint(sign * distance, 0));
                auto* inEffect = qobject_cast<QGraphicsOpacityEffect*>(
                    target->graphicsEffect());
                if (inEffect) {
                    inEffect->setOpacity(0.0);
                }

                auto* in = new QParallelAnimationGroup(this);

                auto* inPos = new QPropertyAnimation(target, "pos", in);
                inPos->setDuration(half);
                inPos->setStartValue(target->pos());
                inPos->setEndValue(origin);
                inPos->setEasingCurve(QEasingCurve::OutCubic);

                if (inEffect) {
                    auto* inFade = new QPropertyAnimation(inEffect, "opacity", in);
                    inFade->setDuration(half);
                    inFade->setStartValue(0.0);
                    inFade->setEndValue(1.0);
                    inFade->setEasingCurve(QEasingCurve::OutCubic);
                }

                m_swapAnimation = in;
                m_swapWidget = target;
                m_swapOrigin = origin;

                connect(in, &QParallelAnimationGroup::finished, in,
                        &QObject::deleteLater);
                connect(in, &QParallelAnimationGroup::finished, this,
                        [this, weakWidget, onFinished, origin, owned]() {
                            if (!weakWidget.isNull()) {
                                QWidget* done = weakWidget.data();
                                done->move(origin);
                                if (auto* fx = qobject_cast<QGraphicsOpacityEffect*>(
                                        done->graphicsEffect())) {
                                    fx->setOpacity(1.0);
                                    if (owned) {
                                        done->setGraphicsEffect(nullptr);
                                    }
                                }
                            }
                            m_swapAnimation.clear();
                            m_swapWidget.clear();
                            if (onFinished) {
                                onFinished();
                            }
                        });

                in->start();
            });

    connect(out, &QParallelAnimationGroup::finished, out, &QObject::deleteLater);

    out->start();
}
