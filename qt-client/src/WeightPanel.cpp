#include "WeightPanel.h"

#include <cmath>

#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

static const QString sliderLabels[] = {"Strongly disagree", "Disagree", "Neutral", "Agree", "Strongly agree"};

double WeightPanel::sliderToMultiplier(int value)
{
    static const double multipliers[] = {0.5, 0.75, 1.0, 1.5, 2.0};
    return multipliers[value];
}

// Finds the closest slider position rather than requiring an exact match, in case a preset's value drifts
int WeightPanel::multiplierToSlider(double multiplier)
{
    static const double multipliers[] = {0.5, 0.75, 1.0, 1.5, 2.0};

    int closest = 0;
    double closestDiff = std::abs(multiplier - multipliers[0]);

    for (int i = 1; i < 5; i++)
    {
        double diff = std::abs(multiplier - multipliers[i]);

        if (diff < closestDiff)
        {
            closestDiff = diff;
            closest = i;
        }
    }

    return closest;
}

QWidget* WeightPanel::buildRow(const QString& question, QSlider*& sliderOut, QWidget* parent)
{
    QWidget* row = new QWidget(parent);

    QLabel* questionLabel = new QLabel(question, row);
    QLabel* valueLabel = new QLabel(sliderLabels[2], row);
    valueLabel->setAlignment(Qt::AlignCenter);

    QSlider* slider = new QSlider(Qt::Horizontal, row);
    slider->setRange(0, 4);
    slider->setValue(2);
    slider->setTickPosition(QSlider::TicksBelow);
    slider->setTickInterval(1);

    QObject::connect(slider, &QSlider::valueChanged, valueLabel, [valueLabel](int value)
    {
        valueLabel->setText(sliderLabels[value]);
    });

    QVBoxLayout* layout = new QVBoxLayout(row);
    layout->addWidget(questionLabel);
    layout->addWidget(slider);
    layout->addWidget(valueLabel);

    sliderOut = slider;
    return row;
}

WeightPanel::WeightPanel(QWidget* parent)
    : QWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(buildRow("I HATE pressing down hard on the strings", stringWeightSlider, this));
    layout->addWidget(buildRow("I HATE stretching for a chord", fretStretchSlider, this));
    layout->addWidget(buildRow("I HATE big jumps between chords", transitionSlider, this));
    layout->addWidget(buildRow("I HATE playing high up the neck", fretPositionSlider, this));
    layout->addWidget(buildRow("I HATE muting or skipping strings", stringSkipSlider, this));
}

WeightValues WeightPanel::weights() const
{
    WeightValues values;
    values.stringWeight = sliderToMultiplier(stringWeightSlider->value());
    values.fretStretch = sliderToMultiplier(fretStretchSlider->value());
    values.transition = sliderToMultiplier(transitionSlider->value());
    values.fretPosition = sliderToMultiplier(fretPositionSlider->value());
    values.stringSkip = sliderToMultiplier(stringSkipSlider->value());

    return values;
}

void WeightPanel::setWeights(const WeightValues& weights)
{
    stringWeightSlider->setValue(multiplierToSlider(weights.stringWeight));
    fretStretchSlider->setValue(multiplierToSlider(weights.fretStretch));
    transitionSlider->setValue(multiplierToSlider(weights.transition));
    fretPositionSlider->setValue(multiplierToSlider(weights.fretPosition));
    stringSkipSlider->setValue(multiplierToSlider(weights.stringSkip));
}
