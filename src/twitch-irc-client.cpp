#include "twitch-irc-client.hpp"

TwitchIrcClient::TwitchIrcClient(QObject *parent)
    : QObject(parent)
    , m_webSocket(new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this))
    , m_pingTimer(new QTimer(this))
    , m_connectionState(ConnectionState::Disconnected)
{
    connect(m_webSocket, &QWebSocket::connected, this, &TwitchIrcClient::onConnected);
    connect(m_webSocket, &QWebSocket::disconnected, this, &TwitchIrcClient::onDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived, this, &TwitchIrcClient::onTextMessageReceived);
    connect(m_webSocket, &QWebSocket::errorOccurred, this, &TwitchIrcClient::onError);
    connect(m_pingTimer, &QTimer::timeout, this, &TwitchIrcClient::sendPing);
    
    // Ping every 4 minutes to keep connection alive
    m_pingTimer->setInterval(4 * 60 * 1000);
}

TwitchIrcClient::~TwitchIrcClient()
{
    disconnect();
}

void TwitchIrcClient::connectToTwitch(const QString &channel, const QString &username, const QString &oauthToken)
{
    if (m_connectionState == ConnectionState::Connected || m_connectionState == ConnectionState::Connecting) {
        disconnect();
    }
    
    m_channel = channel.toLower();
    if (!m_channel.startsWith('#')) {
        m_channel.prepend('#');
    }
    m_username = username.toLower();
    m_oauthToken = oauthToken;
    
    setConnectionState(ConnectionState::Connecting);
    m_webSocket->open(QUrl(QStringLiteral("wss://irc-ws.chat.twitch.tv:443")));
}

void TwitchIrcClient::disconnect()
{
    m_pingTimer->stop();
    if (m_webSocket->state() != QAbstractSocket::UnconnectedState) {
        m_webSocket->close();
    }
    setConnectionState(ConnectionState::Disconnected);
}

void TwitchIrcClient::setConnectionState(ConnectionState state)
{
    if (m_connectionState != state) {
        m_connectionState = state;
        emit connectionStateChanged(state);
    }
}

void TwitchIrcClient::onConnected()
{
    // Request capabilities for tags and commands
    sendRaw(QStringLiteral("CAP REQ :twitch.tv/tags twitch.tv/commands"));
    
    // Authenticate
    sendRaw(QStringLiteral("PASS oauth:%1").arg(m_oauthToken));
    sendRaw(QStringLiteral("NICK %1").arg(m_username));
    
    // Join channel
    sendRaw(QStringLiteral("JOIN %1").arg(m_channel));
    
    setConnectionState(ConnectionState::Connected);
    m_pingTimer->start();
    emit connected();
}

void TwitchIrcClient::onDisconnected()
{
    m_pingTimer->stop();
    setConnectionState(ConnectionState::Disconnected);
    emit disconnected();
}

void TwitchIrcClient::onTextMessageReceived(const QString &message)
{
    QStringList lines = message.split(QStringLiteral("\r\n"), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        parseIrcMessage(line);
    }
}

void TwitchIrcClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)
    m_lastError = m_webSocket->errorString();
    setConnectionState(ConnectionState::Error);
    emit connectionError(m_lastError);
}

void TwitchIrcClient::sendPing()
{
    sendRaw(QStringLiteral("PING :tmi.twitch.tv"));
}

void TwitchIrcClient::sendRaw(const QString &message)
{
    m_webSocket->sendTextMessage(message);
}

void TwitchIrcClient::parseIrcMessage(const QString &rawMessage)
{
    // Handle PING from server
    if (rawMessage.startsWith(QStringLiteral("PING"))) {
        sendRaw(QStringLiteral("PONG :tmi.twitch.tv"));
        return;
    }
    
    // Parse IRCv3 message with tags
    // Format: @tags :prefix COMMAND params :trailing
    
    QString tags;
    QString prefix;
    QString command;
    QString params;
    QString trailing;
    
    QString msg = rawMessage;
    
    // Extract tags if present
    if (msg.startsWith('@')) {
        int spaceIndex = msg.indexOf(' ');
        if (spaceIndex != -1) {
            tags = msg.mid(1, spaceIndex - 1);
            msg = msg.mid(spaceIndex + 1);
        }
    }
    
    // Extract prefix if present
    if (msg.startsWith(':')) {
        int spaceIndex = msg.indexOf(' ');
        if (spaceIndex != -1) {
            prefix = msg.mid(1, spaceIndex - 1);
            msg = msg.mid(spaceIndex + 1);
        }
    }
    
    // Extract command and params
    int trailingIndex = msg.indexOf(QStringLiteral(" :"));
    if (trailingIndex != -1) {
        trailing = msg.mid(trailingIndex + 2);
        msg = msg.left(trailingIndex);
    }
    
    QStringList parts = msg.split(' ', Qt::SkipEmptyParts);
    if (!parts.isEmpty()) {
        command = parts.takeFirst();
        params = parts.join(' ');
    }
    
    // Handle PRIVMSG
    if (command == QStringLiteral("PRIVMSG")) {
        QString username;
        QString displayName;
        
        // Parse tags for display-name
        if (!tags.isEmpty()) {
            QStringList tagList = tags.split(';');
            for (const QString &tag : tagList) {
                int eqIndex = tag.indexOf('=');
                if (eqIndex != -1) {
                    QString key = tag.left(eqIndex);
                    QString value = tag.mid(eqIndex + 1);
                    if (key == QStringLiteral("display-name")) {
                        displayName = value;
                    }
                }
            }
        }
        
        // Extract username from prefix (nick!user@host)
        if (!prefix.isEmpty()) {
            int exclamIndex = prefix.indexOf('!');
            if (exclamIndex != -1) {
                username = prefix.left(exclamIndex);
            } else {
                username = prefix;
            }
        }
        
        // If display-name is empty, use username
        if (displayName.isEmpty()) {
            displayName = username;
        }
        
        if (!username.isEmpty()) {
            emit messageReceived(username.toLower(), displayName, trailing);
        }
    }
}
