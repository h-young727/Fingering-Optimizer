#pragma once

#include <QWidget>

#include "Models.h"

class QLabel;
class QSlider;

class WeightPanel : public QWidget
{
    Q_OBJECT

public:
    explicit WeightPanel(QWidget* parent = nullptr);

    WeightValues weights() const;
    void setWeights(const WeightValues& weights);

private:
    QSlider* stringWeightSlider;
    QSlider* fretStretchSlider;
    QSlider* transitionSlider;
    QSlider* fretPositionSlider;
    QSlider* stringSkipSlider;

    static double sliderToMultiplier(int value);
    static int multiplierToSlider(double multiplier);
    static QWidget* buildRow(const QString& question, QSlider*& sliderOut, QWidget* parent);
};
