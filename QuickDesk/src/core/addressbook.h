// Copyright 2026 QuickDesk Authors
// Адресная книга: книги и записи с сервера (qd-contacts), подключение без ввода пароля.

#ifndef QUICKDESK_CORE_ADDRESSBOOK_H
#define QUICKDESK_CORE_ADDRESSBOOK_H

#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QVariantList>

#include <functional>

namespace quickdesk {

class AddressBookManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList books READ books NOTIFY booksChanged)
    Q_PROPERTY(QVariantList entries READ entries NOTIFY entriesChanged)
    Q_PROPERTY(int currentBook READ currentBook NOTIFY currentBookChanged)
    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
    Q_PROPERTY(bool loggedIn READ loggedIn NOTIFY loggedInChanged)

public:
    explicit AddressBookManager(QObject* parent = nullptr);

    QVariantList books() const { return m_books; }
    QVariantList entries() const { return m_entries; }
    int currentBook() const { return m_currentBook; }
    bool loading() const { return m_loading; }
    QString lastError() const { return m_lastError; }
    bool loggedIn() const;

    Q_INVOKABLE void reload();
    Q_INVOKABLE void selectBook(int bookId);
    Q_INVOKABLE void connectEntry(int entryId);

signals:
    void booksChanged();
    void entriesChanged();
    void currentBookChanged();
    void loadingChanged();
    void lastErrorChanged();
    void loggedInChanged();

    /// Готово к подключению: код получен с сервера, в интерфейсе не показывается.
    void connectRequested(const QString& deviceId, const QString& accessCode);

private:
    QString baseUrl() const;
    QString token() const;
    void setLoading(bool value);
    void setError(const QString& text);
    void request(const QString& path, std::function<void(const QJsonObject&)> onSuccess);

    QNetworkAccessManager m_network;
    QVariantList m_books;
    QVariantList m_entries;
    int m_currentBook = 0;
    bool m_loading = false;
    QString m_lastError;
};

}  // namespace quickdesk

#endif  // QUICKDESK_CORE_ADDRESSBOOK_H
