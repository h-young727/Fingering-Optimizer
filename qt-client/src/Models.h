#pragma once

#include <QString>

struct PieceInfo
{
    int id;
    QString filename;
    QString uploadedAt;
};

struct WeightValues
{
    double stringWeight = 1.0;
    double fretStretch = 1.0;
    double transition = 1.0;
    double fretPosition = 1.0;
    double stringSkip = 1.0;
};

struct TrackResultInfo
{
    int trackIndex;
    QString tabText;
};

struct FingeringResultInfo
{
    int trackIndex;
    QString tabText;
    QString runId;
    QString createdAt;
};

struct PresetInfo
{
    int id;
    QString name;
    WeightValues weights;
};

struct LabeledTab
{
    QString label;
    QString tabText;
};
