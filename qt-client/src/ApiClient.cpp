#include "ApiClient.h"

#include <QFile>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringList>
#include <QUrl>

#include "Config.h"

ApiClient::ApiClient(QObject* parent)
    : QObject(parent)
{
}

QString ApiClient::token() const
{
    return authToken;
}

void ApiClient::logout()
{
    authToken.clear();
    emit loggedOut();
}

// FastAPI validation errors send "detail" as an array instead of a string
static QString extractErrorDetail(QNetworkReply* reply)
{
    QJsonValue detail = QJsonDocument::fromJson(reply->readAll()).object()["detail"];

    if (detail.isString())
    {
        return detail.toString();
    }

    if (detail.isArray())
    {
        QStringList messages;

        for (const QJsonValue& entry : detail.toArray())
        {
            messages << entry.toObject()["msg"].toString();
        }

        QString joined = messages.join("; ");

        if (!joined.isEmpty())
        {
            return joined;
        }
    }

    return reply->errorString();
}

static PieceInfo parsePiece(const QJsonObject& obj)
{
    return {obj["id"].toInt(), obj["filename"].toString(), obj["uploaded_at"].toString()};
}

static WeightValues parseWeightValues(const QJsonObject& obj)
{
    WeightValues weights;
    weights.stringWeight = obj["string_weight"].toDouble();
    weights.fretStretch = obj["fret_stretch"].toDouble();
    weights.transition = obj["transition"].toDouble();
    weights.fretPosition = obj["fret_position"].toDouble();
    weights.stringSkip = obj["string_skip"].toDouble();

    return weights;
}

static QJsonObject weightsToJson(const WeightValues& weights)
{
    QJsonObject obj;
    obj["string_weight"] = weights.stringWeight;
    obj["fret_stretch"] = weights.fretStretch;
    obj["transition"] = weights.transition;
    obj["fret_position"] = weights.fretPosition;
    obj["string_skip"] = weights.stringSkip;

    return obj;
}

static PresetInfo parsePreset(const QJsonObject& obj)
{
    return {obj["id"].toInt(), obj["name"].toString(), parseWeightValues(obj)};
}

void ApiClient::login(const QString& email, const QString& password)
{
    QJsonObject body;
    body["email"] = email;
    body["password"] = password;

    QNetworkRequest request(QUrl(apiBaseUrl + "/auth/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = networkManager.post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]()
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            emit loginFailed(extractErrorDetail(reply));
            reply->deleteLater();
            return;
        }

        QJsonObject response = QJsonDocument::fromJson(reply->readAll()).object();
        authToken = response["access_token"].toString();
        emit loginSucceeded();
        reply->deleteLater();
    });
}

void ApiClient::signup(const QString& email, const QString& password)
{
    QJsonObject body;
    body["email"] = email;
    body["password"] = password;

    QNetworkRequest request(QUrl(apiBaseUrl + "/auth/signup"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply = networkManager.post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]()
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            emit signupFailed(extractErrorDetail(reply));
            reply->deleteLater();
            return;
        }

        emit signupSucceeded();
        reply->deleteLater();
    });
}

void ApiClient::listPieces()
{
    QNetworkRequest request(QUrl(apiBaseUrl + "/pieces"));
    request.setRawHeader("Authorization", ("Bearer " + authToken).toUtf8());

    QNetworkReply* reply = networkManager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]()
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            emit listPiecesFailed(extractErrorDetail(reply));
            reply->deleteLater();
            return;
        }

        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        QList<PieceInfo> pieces;

        for (const QJsonValue& value : arr)
        {
            pieces.append(parsePiece(value.toObject()));
        }

        emit piecesListed(pieces);
        reply->deleteLater();
    });
}

void ApiClient::uploadPiece(const QString& filePath)
{
    QFile* file = new QFile(filePath);

    if (!file->open(QIODevice::ReadOnly))
    {
        emit uploadFailed("Could not open file: " + filePath);
        delete file;
        return;
    }

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QFileInfo fileInfo(filePath);
    QHttpPart filePart;
    filePart.setHeader(
        QNetworkRequest::ContentDispositionHeader,
        QVariant(QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileInfo.fileName())));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("audio/midi"));
    filePart.setBodyDevice(file);
    file->setParent(multiPart);
    multiPart->append(filePart);

    QNetworkRequest request(QUrl(apiBaseUrl + "/pieces"));
    request.setRawHeader("Authorization", ("Bearer " + authToken).toUtf8());

    QNetworkReply* reply = networkManager.post(request, multiPart);
    multiPart->setParent(reply);

    connect(reply, &QNetworkReply::finished, this, [this, reply]()
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            emit uploadFailed(extractErrorDetail(reply));
            reply->deleteLater();
            return;
        }

        emit uploadSucceeded(parsePiece(QJsonDocument::fromJson(reply->readAll()).object()));
        reply->deleteLater();
    });
}

void ApiClient::optimizePiece(int pieceId, const WeightValues& weights)
{
    QJsonObject body;
    body["weights"] = weightsToJson(weights);

    QNetworkRequest request(QUrl(apiBaseUrl + QString("/pieces/%1/optimize").arg(pieceId)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + authToken).toUtf8());

    QNetworkReply* reply = networkManager.post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]()
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            emit optimizeFailed(extractErrorDetail(reply));
            reply->deleteLater();
            return;
        }

        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        QList<TrackResultInfo> results;

        for (const QJsonValue& value : arr)
        {
            QJsonObject obj = value.toObject();
            results.append({obj["track_index"].toInt(), obj["tab_text"].toString()});
        }

        emit optimizeSucceeded(results);
        reply->deleteLater();
    });
}

void ApiClient::listResults(int pieceId)
{
    QNetworkRequest request(QUrl(apiBaseUrl + QString("/pieces/%1/results").arg(pieceId)));
    request.setRawHeader("Authorization", ("Bearer " + authToken).toUtf8());

    QNetworkReply* reply = networkManager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]()
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            emit listResultsFailed(extractErrorDetail(reply));
            reply->deleteLater();
            return;
        }

        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        QList<FingeringResultInfo> results;

        for (const QJsonValue& value : arr)
        {
            QJsonObject obj = value.toObject();
            results.append({obj["track_index"].toInt(), obj["tab_text"].toString(), obj["run_id"].toString(),
                obj["created_at"].toString()});
        }

        emit resultsListed(results);
        reply->deleteLater();
    });
}

void ApiClient::listPresets()
{
    QNetworkRequest request(QUrl(apiBaseUrl + "/presets"));
    request.setRawHeader("Authorization", ("Bearer " + authToken).toUtf8());

    QNetworkReply* reply = networkManager.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]()
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            emit listPresetsFailed(extractErrorDetail(reply));
            reply->deleteLater();
            return;
        }

        QJsonArray arr = QJsonDocument::fromJson(reply->readAll()).array();
        QList<PresetInfo> presets;

        for (const QJsonValue& value : arr)
        {
            presets.append(parsePreset(value.toObject()));
        }

        emit presetsListed(presets);
        reply->deleteLater();
    });
}

void ApiClient::createPreset(const QString& name, const WeightValues& weights)
{
    QJsonObject body = weightsToJson(weights);
    body["name"] = name;

    QNetworkRequest request(QUrl(apiBaseUrl + "/presets"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", ("Bearer " + authToken).toUtf8());

    QNetworkReply* reply = networkManager.post(request, QJsonDocument(body).toJson());

    connect(reply, &QNetworkReply::finished, this, [this, reply]()
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            emit presetCreateFailed(extractErrorDetail(reply));
            reply->deleteLater();
            return;
        }

        emit presetCreated(parsePreset(QJsonDocument::fromJson(reply->readAll()).object()));
        reply->deleteLater();
    });
}

void ApiClient::deletePreset(int presetId)
{
    QNetworkRequest request(QUrl(apiBaseUrl + QString("/presets/%1").arg(presetId)));
    request.setRawHeader("Authorization", ("Bearer " + authToken).toUtf8());

    QNetworkReply* reply = networkManager.deleteResource(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, presetId]()
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            emit presetDeleteFailed(extractErrorDetail(reply));
            reply->deleteLater();
            return;
        }

        emit presetDeleted(presetId);
        reply->deleteLater();
    });
}
