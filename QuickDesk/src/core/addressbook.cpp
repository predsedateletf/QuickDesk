// Copyright 2026 QuickDesk Authors
#include "addressbook.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

#include "../infra/log/log.h"
#include "localconfigcenter.h"

namespace quickdesk {

AddressBookManager::AddressBookManager(QObject* parent)
    : QObject(parent) {}

bool AddressBookManager::loggedIn() const {
    return !token().isEmpty();
}

QString AddressBookManager::token() const {
    return core::LocalConfigCenter::instance().userToken();
}

QString AddressBookManager::baseUrl() const {
    // Сервер сигналинга задан как wss://host — адресная книга живёт на том же хосте.
    QString url = core::LocalConfigCenter::instance().signalingServerUrl();
    url.replace("wss://", "https://");
    url.replace("ws://", "http://");
    while (url.endsWith('/')) url.chop(1);
    return url;
}

void AddressBookManager::setLoading(bool value) {
    if (m_loading == value) return;
    m_loading = value;
    emit loadingChanged();
}

void AddressBookManager::setError(const QString& text) {
    m_lastError = text;
    emit lastErrorChanged();
    if (!text.isEmpty()) LOG_WARN("адресная книга: {}", text.toStdString());
}

void AddressBookManager::request(const QString& path, std::function<void(const QJsonObject&)> onSuccess) {
    const QString bearer = token();
    if (bearer.isEmpty()) {
        setLoading(false);
        setError(tr("Нужно войти в аккаунт"));
        return;
    }
    QNetworkRequest req{QUrl(baseUrl() + path)};
    req.setRawHeader("Authorization", ("Bearer " + bearer).toUtf8());
    req.setTransferTimeout(15000);

    QNetworkReply* reply = m_network.get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, onSuccess]() {
        reply->deleteLater();
        setLoading(false);
        if (reply->error() != QNetworkReply::NoError) {
            setError(reply->errorString());
            return;
        }
        const QJsonObject object = QJsonDocument::fromJson(reply->readAll()).object();
        setError(QString());
        onSuccess(object);
    });
}

void AddressBookManager::reload() {
    if (!loggedIn()) {
        emit loggedInChanged();
        setError(tr("Нужно войти в аккаунт"));
        return;
    }
    setLoading(true);
    request("/v1/me/books", [this](const QJsonObject& object) {
        m_books.clear();
        const QJsonArray items = object.value("items").toArray();
        for (const auto& value : items) {
            const QJsonObject book = value.toObject();
            QVariantMap map;
            map.insert("id", book.value("id").toInt());
            map.insert("name", book.value("name").toString());
            map.insert("contacts", book.value("contacts").toInt());
            map.insert("isOwner", book.value("is_owner").toBool());
            m_books.append(map);
        }
        emit booksChanged();
        if (m_currentBook == 0 && !m_books.isEmpty()) {
            selectBook(m_books.first().toMap().value("id").toInt());
        } else if (m_currentBook != 0) {
            selectBook(m_currentBook);
        }
    });
}

void AddressBookManager::selectBook(int bookId) {
    if (bookId <= 0) return;
    const bool changed = m_currentBook != bookId;
    m_currentBook = bookId;
    if (changed) emit currentBookChanged();

    setLoading(true);
    request(QString("/v1/me/books/%1/contacts").arg(bookId), [this](const QJsonObject& object) {
        m_entries.clear();
        const QJsonArray items = object.value("items").toArray();
        for (const auto& value : items) {
            const QJsonObject entry = value.toObject();
            QVariantMap map;
            map.insert("id", entry.value("id").toInt());
            map.insert("name", entry.value("name").toString());
            map.insert("deviceId", entry.value("device_id").toString());
            map.insert("note", entry.value("note").toString());
            map.insert("hasPassword", !entry.value("access_code").toString().isEmpty());
            m_entries.append(map);
        }
        emit entriesChanged();
    });
}

void AddressBookManager::connectEntry(int entryId) {
    if (m_currentBook <= 0) return;
    setLoading(true);
    request(QString("/v1/me/books/%1/contacts/%2/credentials").arg(m_currentBook).arg(entryId),
            [this](const QJsonObject& object) {
        const QString deviceId = object.value("device_id").toString();
        const QString accessCode = object.value("access_code").toString();
        if (deviceId.isEmpty() || accessCode.isEmpty()) {
            setError(tr("У записи не задан пароль — обратитесь к администратору"));
            return;
        }
        emit connectRequested(deviceId, accessCode);
    });
}

}  // namespace quickdesk
