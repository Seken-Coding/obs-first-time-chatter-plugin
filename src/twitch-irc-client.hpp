#pragma once

#include <QObject>
#include <QWebSocket>
#include <QTimer>
#include <QString>

class TwitchIrcClient : public QObject {
    Q_OBJECT

public:
    enum class ConnectionState {
        Disconnected,
        Connecting,
        Connected,
        Error
    };

    explicit TwitchIrcClient(QObject *parent = nullptr);
    ~TwitchIrcClient() override;

    void connectToTwitch(const QString &channel, const QString &username, const QString &oauthToken);
    void disconnect();
    
    ConnectionState connectionState() const { return m_connectionState; }
    QString lastError() const { return m_lastError; }

signals:
    void connected();
    void disconnected();
    void messageReceived(const QString &username, const QString &displayName, const QString &message);
    void connectionError(const QString &error);
    void connectionStateChanged(ConnectionState state);

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void onError(QAbstractSocket::SocketError error);
    void sendPing();

private:
    void setConnectionState(ConnectionState state);
    void parseIrcMessage(const QString &rawMessage);
    void sendRaw(const QString &message);

    QWebSocket *m_webSocket;
    QTimer *m_pingTimer;
    QString m_channel;
    QString m_username;
    QString m_oauthToken;
    ConnectionState m_connectionState;
    QString m_lastError;
};
