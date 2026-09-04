#pragma once

#include <QList>
#include <QWidget>

#include "Models.h"

class ApiClient;
class QComboBox;
class QLabel;
class QListWidget;
class WeightPanel;

class LibraryView : public QWidget
{
    Q_OBJECT

public:
    explicit LibraryView(ApiClient* apiClient, QWidget* parent = nullptr);

    void refresh();

private:
    ApiClient* apiClient;
    QListWidget* pieceListWidget;
    WeightPanel* weightPanel;
    QComboBox* presetComboBox;
    QLabel* statusLabel;
    QList<PresetInfo> presets;

    void handleUploadClicked();
    void handleOptimizeClicked();
    void handleViewHistoryClicked();
    void handleLoadPresetClicked();
    void handleSavePresetClicked();
    void handleDeletePresetClicked();
    void handleLogoutClicked();
    void showTabDialog(const QList<LabeledTab>& tabs, const QString& title);
};
