#include "LibraryView.h"

#include <QComboBox>
#include <QDialog>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>

#include "ApiClient.h"
#include "TabResultView.h"
#include "WeightPanel.h"

LibraryView::LibraryView(ApiClient* apiClient, QWidget* parent)
    : QWidget(parent)
    , apiClient(apiClient)
{
    pieceListWidget = new QListWidget(this);
    weightPanel = new WeightPanel(this);
    presetComboBox = new QComboBox(this);
    statusLabel = new QLabel(this);

    QPushButton* uploadButton = new QPushButton("Upload MIDI", this);
    QPushButton* optimizeButton = new QPushButton("Optimize Selected Piece", this);
    QPushButton* historyButton = new QPushButton("View History", this);
    QPushButton* loadPresetButton = new QPushButton("Load", this);
    QPushButton* savePresetButton = new QPushButton("Save As...", this);
    QPushButton* deletePresetButton = new QPushButton("Delete", this);
    QPushButton* logoutButton = new QPushButton("Log Out", this);

    QVBoxLayout* leftColumn = new QVBoxLayout();
    leftColumn->addWidget(new QLabel("Your Pieces", this));
    leftColumn->addWidget(pieceListWidget);
    leftColumn->addWidget(uploadButton);

    QHBoxLayout* pieceActionsLayout = new QHBoxLayout();
    pieceActionsLayout->addWidget(optimizeButton);
    pieceActionsLayout->addWidget(historyButton);
    leftColumn->addLayout(pieceActionsLayout);

    QHBoxLayout* presetLayout = new QHBoxLayout();
    presetLayout->addWidget(presetComboBox);
    presetLayout->addWidget(loadPresetButton);
    presetLayout->addWidget(savePresetButton);
    presetLayout->addWidget(deletePresetButton);

    QVBoxLayout* rightColumn = new QVBoxLayout();
    rightColumn->addWidget(logoutButton);
    rightColumn->addWidget(new QLabel("Weight Presets", this));
    rightColumn->addLayout(presetLayout);
    rightColumn->addWidget(weightPanel);
    rightColumn->addWidget(statusLabel);
    rightColumn->addStretch();

    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->addLayout(leftColumn);
    layout->addLayout(rightColumn);

    connect(uploadButton, &QPushButton::clicked, this, &LibraryView::handleUploadClicked);
    connect(optimizeButton, &QPushButton::clicked, this, &LibraryView::handleOptimizeClicked);
    connect(historyButton, &QPushButton::clicked, this, &LibraryView::handleViewHistoryClicked);
    connect(loadPresetButton, &QPushButton::clicked, this, &LibraryView::handleLoadPresetClicked);
    connect(savePresetButton, &QPushButton::clicked, this, &LibraryView::handleSavePresetClicked);
    connect(deletePresetButton, &QPushButton::clicked, this, &LibraryView::handleDeletePresetClicked);
    connect(logoutButton, &QPushButton::clicked, this, &LibraryView::handleLogoutClicked);

    connect(apiClient, &ApiClient::piecesListed, this, [this](const QList<PieceInfo>& pieces)
    {
        pieceListWidget->clear();

        for (const PieceInfo& piece : pieces)
        {
            QListWidgetItem* item = new QListWidgetItem(piece.filename, pieceListWidget);
            item->setData(Qt::UserRole, piece.id);
        }
    });

    connect(apiClient, &ApiClient::listPiecesFailed, this, [this](const QString& message)
    {
        statusLabel->setText(message);
    });

    connect(apiClient, &ApiClient::uploadSucceeded, this, [this](const PieceInfo&)
    {
        statusLabel->setText("Upload succeeded");
        refresh();
    });

    connect(apiClient, &ApiClient::uploadFailed, this, [this](const QString& message)
    {
        statusLabel->setText(message);
    });

    connect(apiClient, &ApiClient::optimizeSucceeded, this, [this](const QList<TrackResultInfo>& results)
    {
        statusLabel->setText("");

        QList<LabeledTab> tabs;

        for (const TrackResultInfo& result : results)
        {
            tabs.append({QString("Track %1").arg(result.trackIndex), result.tabText});
        }

        showTabDialog(tabs, "Fingering Results");
    });

    connect(apiClient, &ApiClient::optimizeFailed, this, [this](const QString& message)
    {
        statusLabel->setText(message);
    });

    connect(apiClient, &ApiClient::resultsListed, this, [this](const QList<FingeringResultInfo>& results)
    {
        statusLabel->setText("");

        QList<LabeledTab> tabs;

        for (const FingeringResultInfo& result : results)
        {
            tabs.append({QString("Track %1 (%2)").arg(result.trackIndex).arg(result.createdAt), result.tabText});
        }

        showTabDialog(tabs, "Past Results");
    });

    connect(apiClient, &ApiClient::listResultsFailed, this, [this](const QString& message)
    {
        statusLabel->setText(message);
    });

    connect(apiClient, &ApiClient::presetsListed, this, [this](const QList<PresetInfo>& listedPresets)
    {
        presets = listedPresets;
        presetComboBox->clear();

        for (const PresetInfo& preset : presets)
        {
            presetComboBox->addItem(preset.name);
        }
    });

    connect(apiClient, &ApiClient::listPresetsFailed, this, [this](const QString& message)
    {
        statusLabel->setText(message);
    });

    connect(apiClient, &ApiClient::presetCreated, this, [this](const PresetInfo&)
    {
        statusLabel->setText("Preset saved");
        this->apiClient->listPresets();
    });

    connect(apiClient, &ApiClient::presetCreateFailed, this, [this](const QString& message)
    {
        statusLabel->setText(message);
    });

    connect(apiClient, &ApiClient::presetDeleted, this, [this](int)
    {
        statusLabel->setText("Preset deleted");
        this->apiClient->listPresets();
    });

    connect(apiClient, &ApiClient::presetDeleteFailed, this, [this](const QString& message)
    {
        statusLabel->setText(message);
    });

    connect(apiClient, &ApiClient::loggedOut, this, [this]()
    {
        pieceListWidget->clear();
        presetComboBox->clear();
        presets.clear();
        statusLabel->setText("");
    });
}

void LibraryView::refresh()
{
    apiClient->listPieces();
    apiClient->listPresets();
}

void LibraryView::showTabDialog(const QList<LabeledTab>& tabs, const QString& title)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.resize(700, 500);

    TabResultView* resultView = new TabResultView(&dialog);
    resultView->showResults(tabs);

    QVBoxLayout* dialogLayout = new QVBoxLayout(&dialog);
    dialogLayout->addWidget(resultView);

    dialog.exec();
}

void LibraryView::handleUploadClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select MIDI File", QString(), "MIDI Files (*.mid *.midi)");

    if (filePath.isEmpty())
    {
        return;
    }

    statusLabel->setText("Uploading...");
    apiClient->uploadPiece(filePath);
}

void LibraryView::handleOptimizeClicked()
{
    QListWidgetItem* selected = pieceListWidget->currentItem();

    if (selected == nullptr)
    {
        statusLabel->setText("Select a piece first");
        return;
    }

    statusLabel->setText("Optimizing...");
    int pieceId = selected->data(Qt::UserRole).toInt();
    apiClient->optimizePiece(pieceId, weightPanel->weights());
}

void LibraryView::handleViewHistoryClicked()
{
    QListWidgetItem* selected = pieceListWidget->currentItem();

    if (selected == nullptr)
    {
        statusLabel->setText("Select a piece first");
        return;
    }

    statusLabel->setText("Loading history...");
    int pieceId = selected->data(Qt::UserRole).toInt();
    apiClient->listResults(pieceId);
}

void LibraryView::handleLoadPresetClicked()
{
    int index = presetComboBox->currentIndex();

    if (index < 0 || index >= presets.size())
    {
        statusLabel->setText("Select a preset first");
        return;
    }

    weightPanel->setWeights(presets[index].weights);
    statusLabel->setText("Preset loaded");
}

void LibraryView::handleSavePresetClicked()
{
    bool ok = false;
    QString name = QInputDialog::getText(this, "Save Preset", "Preset name:", QLineEdit::Normal, QString(), &ok);

    if (!ok || name.isEmpty())
    {
        return;
    }

    statusLabel->setText("Saving preset...");
    apiClient->createPreset(name, weightPanel->weights());
}

void LibraryView::handleDeletePresetClicked()
{
    int index = presetComboBox->currentIndex();

    if (index < 0 || index >= presets.size())
    {
        statusLabel->setText("Select a preset first");
        return;
    }

    apiClient->deletePreset(presets[index].id);
}

void LibraryView::handleLogoutClicked()
{
    apiClient->logout();
}
