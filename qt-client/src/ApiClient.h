#pragma once

#include <QList>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>

#include "Models.h"

class ApiClient : public QObject
{
    Q_OBJECT

public:
    explicit ApiClient(QObject* parent = nullptr);

    void login(const QString& email, const QString& password);
    void signup(const QString& email, const QString& password);
    void logout();
    void listPieces();
    void uploadPiece(const QString& filePath);
    void optimizePiece(int pieceId, const WeightValues& weights);
    void listResults(int pieceId);
    void listPresets();
    void createPreset(const QString& name, const WeightValues& weights);
    void deletePreset(int presetId);

    QString token() const;

signals:
    void loginSucceeded();
    void loginFailed(const QString& message);
    void signupSucceeded();
    void signupFailed(const QString& message);
    void loggedOut();
    void piecesListed(const QList<PieceInfo>& pieces);
    void listPiecesFailed(const QString& message);
    void uploadSucceeded(const PieceInfo& piece);
    void uploadFailed(const QString& message);
    void optimizeSucceeded(const QList<TrackResultInfo>& results);
    void optimizeFailed(const QString& message);
    void resultsListed(const QList<FingeringResultInfo>& results);
    void listResultsFailed(const QString& message);
    void presetsListed(const QList<PresetInfo>& presets);
    void listPresetsFailed(const QString& message);
    void presetCreated(const PresetInfo& preset);
    void presetCreateFailed(const QString& message);
    void presetDeleted(int presetId);
    void presetDeleteFailed(const QString& message);

private:
    QNetworkAccessManager networkManager;
    QString authToken;
};
