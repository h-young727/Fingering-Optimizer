#pragma once

#include <QList>
#include <QWidget>

#include "Models.h"

class QTabWidget;

class TabResultView : public QWidget
{
    Q_OBJECT

public:
    explicit TabResultView(QWidget* parent = nullptr);

    void showResults(const QList<LabeledTab>& tabs);

private:
    QTabWidget* tabWidget;
};
