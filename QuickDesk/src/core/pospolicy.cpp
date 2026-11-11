// Copyright 2026 QuickDesk Authors
#include "pospolicy.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSettings>
#include <QUrl>
#include <QUrlQuery>

#include "../infra/log/log.h"

namespace quickdesk {

namespace {
constexpr const char* kDefaultPolicyUrl = "https://qd.lyashenko.pw/v1/pos/policy";
constexpr const char* kDefaultEnrollUrl = "https://qd.lyashenko.pw/v1/pos/enroll";
constexpr const char* kDefaultServer = "wss://qd.lyashenko.pw";
// Общий секрет установки: позволяет кассе самой зарегистрироваться при первом
// запуске. Меняется на сервере, тогда новые установки требуют новый секрет.
constexpr const char* kInstallSecret = "Ra_SgDg2sOF5rD81FrHxzFdWv_1r6ixC";
}  // namespace

PosPolicy::PosPolicy(QObject* parent)
    : QObject(parent) {
    m_timer.setInterval(60 * 1000);
    connect(&m_timer, &QTimer::timeout, this, &PosPolicy::poll);
}

bool PosPolicy::isEnabled() {
    return QFile::exists(QCoreApplication::applicationDirPath() + "/pos.ini");
}

static QString iniPath() {
    return QCoreApplication::applicationDirPath() + "/pos.ini";
}

bool PosPolicy::load() {
    QSettings settings(iniPath(), QSettings::IniFormat);
    m_deviceId = settings.value("device_id").toString();
    m_pollToken = settings.value("poll_token").toString();
    m_server = settings.value("server", kDefaultServer).toString();
    m_policyUrl = settings.value("policy_url", kDefaultPolicyUrl).toString();
    m_enrollUrl = settings.value("enroll_url", kDefaultEnrollUrl).toString();
    m_installSecret = settings.value("install_secret", kInstallSecret).toString();
    m_machineName = settings.value("name").toString();
    m_accessCode = settings.value("cached_code").toString();
    m_cachePath = iniPath();
    return true;
}

void PosPolicy::setDeviceId(const QString& deviceId) {
    if (deviceId.isEmpty() || deviceId == m_deviceId) {
        return;
    }
    m_deviceId = deviceId;
    if (m_pollToken.isEmpty()) {
        enroll();
        return;
    }
    poll();
}

void PosPolicy::enroll() {
    if (m_deviceId.isEmpty() || m_requestInFlight) {
        return;
    }
    QJsonObject payload;
    payload.insert("device_id", m_deviceId);
    payload.insert("name", m_machineName);

    QNetworkRequest request(QUrl(m_enrollUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("X-Install-Secret", m_installSecret.toUtf8());
    request.setTransferTimeout(15000);

    m_requestInFlight = true;
    QNetworkReply* reply = m_network.post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        m_requestInFlight = false;
        if (reply->error() != QNetworkReply::NoError) {
            emit policyFailed(reply->errorString());
            reply->deleteLater();
            return;
        }
        const QJsonObject object = QJsonDocument::fromJson(reply->readAll()).object();
        reply->deleteLater();
        const QString token = object.value("poll_token").toString();
        const QString code = object.value("access_code").toString();
        if (token.isEmpty() || code.isEmpty()) {
            emit policyFailed("enroll: empty response");
            return;
        }
        m_pollToken = token;
        m_server = object.value("signaling").toString(m_server);
        const bool changed = code != m_accessCode;
        m_accessCode = code;
        saveCache();
        LOG_INFO("POS: касса зарегистрирована, постоянный пароль получен");
        if (changed) {
            emit accessCodeReady(m_accessCode, m_server);
        }
    });
}

void PosPolicy::start() {
    if (!m_pollToken.isEmpty() && !m_deviceId.isEmpty()) {
        poll();
    }
    m_timer.start();
}

void PosPolicy::poll() {
    if (m_requestInFlight || m_pollToken.isEmpty() || m_deviceId.isEmpty()) {
        return;
    }
    QUrl url(m_policyUrl);
    QUrlQuery query;
    query.addQueryItem("device_id", m_deviceId);
    query.addQueryItem("token", m_pollToken);
    url.setQuery(query);

    QNetworkRequest request(url);
    request.setTransferTimeout(15000);

    m_requestInFlight = true;
    QNetworkReply* reply = m_network.get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        m_requestInFlight = false;
        const int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if (reply->error() != QNetworkReply::NoError) {
            emit policyFailed(reply->errorString());
            reply->deleteLater();
            return;
        }
        handleReply(reply->readAll(), status);
        reply->deleteLater();
    });
}

void PosPolicy::handleReply(const QByteArray& body, int status) {
    if (status != 200) {
        emit policyFailed(QString("HTTP %1").arg(status));
        return;
    }
    const QJsonObject object = QJsonDocument::fromJson(body).object();
    const QString code = object.value("access_code").toString();
    const QString server = object.value("signaling").toString();
    if (code.isEmpty()) {
        emit policyFailed("empty access_code");
        return;
    }
    if (!server.isEmpty()) {
        m_server = server;
    }
    if (code == m_accessCode) {
        return;
    }
    m_accessCode = code;
    saveCache();
    LOG_INFO("POS: получен постоянный пароль кассы, длина {}", code.size());
    emit accessCodeReady(m_accessCode, m_server);
}

bool PosPolicy::saveCache() {
    QSettings settings(iniPath(), QSettings::IniFormat);
    settings.setValue("cached_code", m_accessCode);
    settings.setValue("poll_token", m_pollToken);
    settings.setValue("device_id", m_deviceId);
    settings.sync();
    return true;
}

}  // namespace quickdesk
