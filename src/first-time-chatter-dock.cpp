#include "first-time-chatter-dock.hpp"
#include "settings-dialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QFont>

// Default bot list
static const QStringList DEFAULT_BOTS = {
    "nightbot",
    "streamelements",
    "moobot",
    "fossabot",
    "soundalerts",
    "streamlabs",
    "wizebot",
    "botisimo",
    "coebot",
    "deepbot",
    "phantombot"
};

FirstTimeChatterDock::FirstTimeChatterDock(QWidget *parent)
    : QFrame(parent)
    , m_ircClient(new TwitchIrcClient(this))
{
    setupUi();
    applyDarkTheme();
    loadSettings();
    loadGreetedState();
    checkDailyReset();
    
    connect(m_ircClient, &TwitchIrcClient::connected, this, &FirstTimeChatterDock::onIrcConnected);
    connect(m_ircClient, &TwitchIrcClient::disconnected, this, &FirstTimeChatterDock::onIrcDisconnected);
    connect(m_ircClient, &TwitchIrcClient::messageReceived, this, &FirstTimeChatterDock::onIrcMessage);
    connect(m_ircClient, &TwitchIrcClient::connectionError, this, &FirstTimeChatterDock::onIrcError);
    connect(m_ircClient, &TwitchIrcClient::connectionStateChanged, this, &FirstTimeChatterDock::onConnectionStateChanged);
}

FirstTimeChatterDock::~FirstTimeChatterDock()
{
    saveSettings();
    saveGreetedState();
    m_ircClient->disconnect();
}

void FirstTimeChatterDock::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(8, 8, 8, 8);
    mainLayout->setSpacing(8);
    
    // Status and title row
    auto *headerLayout = new QHBoxLayout();
    
    m_statusLabel = new QLabel(QStringLiteral("⚫ Disconnected"), this);
    headerLayout->addWidget(m_statusLabel);
    
    headerLayout->addStretch();
    
    m_settingsButton = new QPushButton(QStringLiteral("⚙"), this);
    m_settingsButton->setFixedSize(28, 28);
    m_settingsButton->setToolTip(tr("Settings"));
    headerLayout->addWidget(m_settingsButton);
    
    mainLayout->addLayout(headerLayout);
    
    // Chatter list
    m_chatterList = new QListWidget(this);
    m_chatterList->setSelectionMode(QAbstractItemView::NoSelection);
    m_chatterList->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    mainLayout->addWidget(m_chatterList, 1);
    
    // Button row
    auto *buttonLayout = new QHBoxLayout();
    
    m_connectButton = new QPushButton(tr("Connect"), this);
    buttonLayout->addWidget(m_connectButton);
    
    m_resetButton = new QPushButton(tr("Reset"), this);
    buttonLayout->addWidget(m_resetButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(m_connectButton, &QPushButton::clicked, this, &FirstTimeChatterDock::onConnectClicked);
    connect(m_resetButton, &QPushButton::clicked, this, &FirstTimeChatterDock::onResetClicked);
    connect(m_settingsButton, &QPushButton::clicked, this, &FirstTimeChatterDock::onSettingsClicked);
    connect(m_chatterList, &QListWidget::itemClicked, this, &FirstTimeChatterDock::onChatterClicked);
}

void FirstTimeChatterDock::applyDarkTheme()
{
    setStyleSheet(QStringLiteral(
        "QFrame { background-color: #18181b; }"
        "QLabel { color: #efeff1; }"
        "QListWidget { background-color: #0e0e10; color: #efeff1; border: 1px solid #3d3d42; border-radius: 4px; }"
        "QListWidget::item { padding: 6px 8px; border-bottom: 1px solid #2d2d32; }"
        "QListWidget::item:hover { background-color: #26262c; }"
        "QPushButton { background-color: #9147ff; color: #ffffff; border: none; border-radius: 4px; padding: 8px 16px; }"
        "QPushButton:hover { background-color: #772ce8; }"
        "QPushButton:pressed { background-color: #5c16c5; }"
        "QPushButton:disabled { background-color: #3d3d42; color: #7a7a7d; }"
    ));
}

void FirstTimeChatterDock::loadSettings()
{
    QSettings settings(QStringLiteral("FirstTimeChatterDock"), QStringLiteral("OBS"));
    
    m_channel = settings.value(QStringLiteral("channel")).toString();
    m_username = settings.value(QStringLiteral("username")).toString();
    m_oauthToken = settings.value(QStringLiteral("oauthToken")).toString();
    
    QStringList bots = settings.value(QStringLiteral("botList"), DEFAULT_BOTS).toStringList();
    m_botList.clear();
    for (const QString &bot : bots) {
        m_botList.insert(bot.toLower());
    }
    
    m_lastResetDate = settings.value(QStringLiteral("lastResetDate"), QDate::currentDate()).toDate();
}

void FirstTimeChatterDock::saveSettings()
{
    QSettings settings(QStringLiteral("FirstTimeChatterDock"), QStringLiteral("OBS"));
    
    settings.setValue(QStringLiteral("channel"), m_channel);
    settings.setValue(QStringLiteral("username"), m_username);
    settings.setValue(QStringLiteral("oauthToken"), m_oauthToken);
    settings.setValue(QStringLiteral("botList"), QStringList(m_botList.begin(), m_botList.end()));
    settings.setValue(QStringLiteral("lastResetDate"), m_lastResetDate);
}

void FirstTimeChatterDock::loadGreetedState()
{
    QSettings settings(QStringLiteral("FirstTimeChatterDock"), QStringLiteral("OBS"));
    
    QStringList greeted = settings.value(QStringLiteral("greetedChatters")).toStringList();
    m_greetedChatters.clear();
    for (const QString &chatter : greeted) {
        m_greetedChatters.insert(chatter.toLower());
    }
}

void FirstTimeChatterDock::saveGreetedState()
{
    QSettings settings(QStringLiteral("FirstTimeChatterDock"), QStringLiteral("OBS"));
    settings.setValue(QStringLiteral("greetedChatters"), QStringList(m_greetedChatters.begin(), m_greetedChatters.end()));
}

void FirstTimeChatterDock::checkDailyReset()
{
    QDate today = QDate::currentDate();
    if (m_lastResetDate != today) {
        // New day - reset everything
        m_seenChatters.clear();
        m_greetedChatters.clear();
        m_chatterList->clear();
        m_lastResetDate = today;
        saveSettings();
        saveGreetedState();
    }
}

void FirstTimeChatterDock::updateStatusIndicator()
{
    switch (m_ircClient->connectionState()) {
        case TwitchIrcClient::ConnectionState::Connected:
            m_statusLabel->setText(QStringLiteral("🟢 Connected"));
            m_connectButton->setText(tr("Disconnect"));
            break;
        case TwitchIrcClient::ConnectionState::Connecting:
            m_statusLabel->setText(QStringLiteral("🟡 Connecting..."));
            m_connectButton->setText(tr("Cancel"));
            break;
        case TwitchIrcClient::ConnectionState::Error:
            m_statusLabel->setText(QStringLiteral("🔴 Error"));
            m_connectButton->setText(tr("Connect"));
            break;
        case TwitchIrcClient::ConnectionState::Disconnected:
        default:
            m_statusLabel->setText(QStringLiteral("⚫ Disconnected"));
            m_connectButton->setText(tr("Connect"));
            break;
    }
}

void FirstTimeChatterDock::addChatterToList(const QString &displayName, const QString &username)
{
    auto *item = new QListWidgetItem(displayName);
    item->setData(Qt::UserRole, username);
    
    // Check if already greeted
    if (m_greetedChatters.contains(username)) {
        QFont font = item->font();
        font.setStrikeOut(true);
        item->setFont(font);
        item->setForeground(QColor(0x7a, 0x7a, 0x7d)); // Dimmed color
    } else {
        item->setForeground(QColor(0xef, 0xef, 0xf1)); // Normal color
    }
    
    // Insert at top (newest first)
    m_chatterList->insertItem(0, item);
}

bool FirstTimeChatterDock::isBot(const QString &username) const
{
    return m_botList.contains(username.toLower());
}

void FirstTimeChatterDock::onConnectClicked()
{
    if (m_ircClient->connectionState() == TwitchIrcClient::ConnectionState::Connected ||
        m_ircClient->connectionState() == TwitchIrcClient::ConnectionState::Connecting) {
        m_ircClient->disconnect();
    } else {
        // Check daily reset before connecting
        checkDailyReset();
        
        if (m_channel.isEmpty() || m_username.isEmpty() || m_oauthToken.isEmpty()) {
            QMessageBox::warning(this, tr("Configuration Required"),
                tr("Please configure your Twitch channel, username, and OAuth token in settings."));
            onSettingsClicked();
            return;
        }
        m_ircClient->connectToTwitch(m_channel, m_username, m_oauthToken);
    }
}

void FirstTimeChatterDock::onResetClicked()
{
    m_seenChatters.clear();
    m_greetedChatters.clear();
    m_chatterList->clear();
    m_lastResetDate = QDate::currentDate();
    saveSettings();
    saveGreetedState();
}

void FirstTimeChatterDock::onSettingsClicked()
{
    SettingsDialog dialog(this);
    dialog.setChannel(m_channel);
    dialog.setUsername(m_username);
    dialog.setOauthToken(m_oauthToken);
    dialog.setBotList(QStringList(m_botList.begin(), m_botList.end()));
    
    if (dialog.exec() == QDialog::Accepted) {
        m_channel = dialog.channel();
        m_username = dialog.username();
        m_oauthToken = dialog.oauthToken();
        
        m_botList.clear();
        QStringList bots = dialog.botList();
        for (const QString &bot : bots) {
            m_botList.insert(bot.toLower());
        }
        
        saveSettings();
    }
}

void FirstTimeChatterDock::onChatterClicked(QListWidgetItem *item)
{
    QString username = item->data(Qt::UserRole).toString();
    
    if (m_greetedChatters.contains(username)) {
        // Un-greet: remove strikethrough
        m_greetedChatters.remove(username);
        QFont font = item->font();
        font.setStrikeOut(false);
        item->setFont(font);
        item->setForeground(QColor(0xef, 0xef, 0xf1));
    } else {
        // Greet: add strikethrough
        m_greetedChatters.insert(username);
        QFont font = item->font();
        font.setStrikeOut(true);
        item->setFont(font);
        item->setForeground(QColor(0x7a, 0x7a, 0x7d));
    }
    
    saveGreetedState();
}

void FirstTimeChatterDock::onIrcConnected()
{
    updateStatusIndicator();
}

void FirstTimeChatterDock::onIrcDisconnected()
{
    updateStatusIndicator();
}

void FirstTimeChatterDock::onIrcMessage(const QString &username, const QString &displayName, const QString &message)
{
    Q_UNUSED(message)
    
    // Check daily reset
    checkDailyReset();
    
    // Filter bots
    if (isBot(username)) {
        return;
    }
    
    // Check if first time chatter this session
    if (!m_seenChatters.contains(username)) {
        m_seenChatters.insert(username);
        addChatterToList(displayName, username);
    }
}

void FirstTimeChatterDock::onIrcError(const QString &error)
{
    Q_UNUSED(error)
    updateStatusIndicator();
}

void FirstTimeChatterDock::onConnectionStateChanged(TwitchIrcClient::ConnectionState state)
{
    Q_UNUSED(state)
    updateStatusIndicator();
}
