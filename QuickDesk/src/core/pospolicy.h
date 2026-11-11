// Copyright 2026 QuickDesk Authors
// POS policy: постоянный пароль кассы, который хранится на сервере.
//
// Касса при старте получает код доступа у нашего сервиса и далее раз в минуту
// спрашивает его снова: если администратор сменил пароль, станция забирает
// новый и перерегистрирует код у сигналинга.

#ifndef QUICKDESK_CORE_POSPOLICY_H
#define QUICKDESK_CORE_POSPOLICY_H

#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QTimer>

namespace quickdesk {

class PosPolicy : public QObject {
    Q_OBJECT
public:
    explicit PosPolicy(QObject* parent = nullptr);

    /// Включён ли режим кассы: рядом с исполняемым файлом лежит pos.ini.
    static bool isEnabled();

    /// Читает pos.ini (device_id, poll_token, server, policy_url).
    bool load();

    /// Запускает первый опрос и таймер (60 с).
    void start();

    /// Касса узнала свой device_id — регистрируемся или сразу опрашиваем.
    void setDeviceId(const QString& deviceId);

    QString deviceId() const { return m_deviceId; }
    QString server() const { return m_server; }
    QString accessCode() const { return m_accessCode; }

signals:
    /// Получен (или сменился) постоянный пароль кассы.
    void accessCodeReady(const QString& code, const QString& server);
    /// Сервис недоступен или ответил ошибкой — только для лога и статуса.
    void policyFailed(const QString& reason);

private:
    void poll();
    void enroll();
    void handleReply(const QByteArray& body, int status);
    bool saveCache();

    QNetworkAccessManager m_network;
    QTimer m_timer;
    QString m_deviceId;
    QString m_pollToken;
    QString m_server;
    QString m_policyUrl;
    QString m_enrollUrl;
    QString m_installSecret;
    QString m_machineName;
    QString m_accessCode;
    QString m_cachePath;
    bool m_requestInFlight = false;
};

}  // namespace quickdesk

#endif  // QUICKDESK_CORE_POSPOLICY_H
