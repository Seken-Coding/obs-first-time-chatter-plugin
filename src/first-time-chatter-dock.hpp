#pragma once

#include <QFrame>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QSet>
#include <QSettings>
#include <QDate>
#include "twitch-irc-client.hpp"

class FirstTimeChatterDock : public QFrame {
    Q_OBJECT

public:
    explicit FirstTimeChatterDock(QWidget *parent = nullptr);
    ~FirstTimeChatterDock() override;

private slots:
    void onConnectClicked();
    void onResetClicked();
    void onSettingsClicked();
    void onChatterClicked(QListWidgetItem *item);
    void onIrcConnected();
    void onIrcDisconnected();
    void onIrcMessage(const QString &username, const QString &displayName, const QString &message);
    void onIrcError(const QString &error);
    void onConnectionStateChanged(TwitchIrcClient::ConnectionState state);

private:
    void setupUi();
    void applyDarkTheme();
    void loadSettings();
    void saveSettings();
    void loadGreetedState();
    void saveGreetedState();
    void checkDailyReset();
    void updateStatusIndicator();
    void addChatterToList(const QString &displayName, const QString &username);
    bool isBot(const QString &username) const;

    TwitchIrcClient *m_ircClient;
    QListWidget *m_chatterList;
    QPushButton *m_connectButton;
    QPushButton *m_resetButton;
    QPushButton *m_settingsButton;
    QLabel *m_statusLabel;
    
    QSet<QString> m_seenChatters;
    QSet<QString> m_greetedChatters;
    QSet<QString> m_botList;
    
    QString m_channel;
    QString m_username;
    QString m_oauthToken;
    
    QDate m_lastResetDate;
};
