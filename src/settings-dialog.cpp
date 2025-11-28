#include "settings-dialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QGroupBox>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUi();
    applyDarkTheme();
    setWindowTitle(tr("First-Time Chatter Settings"));
    setMinimumWidth(400);
}

void SettingsDialog::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    
    // Twitch connection group
    auto *connectionGroup = new QGroupBox(tr("Twitch Connection"), this);
    auto *connectionLayout = new QFormLayout(connectionGroup);
    
    m_channelEdit = new QLineEdit(this);
    m_channelEdit->setPlaceholderText(tr("e.g., yourchannelname"));
    connectionLayout->addRow(tr("Channel:"), m_channelEdit);
    
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText(tr("Your Twitch username"));
    connectionLayout->addRow(tr("Username:"), m_usernameEdit);
    
    m_oauthEdit = new QLineEdit(this);
    m_oauthEdit->setEchoMode(QLineEdit::Password);
    m_oauthEdit->setPlaceholderText(tr("oauth:xxxxxxxxxxxxxx"));
    connectionLayout->addRow(tr("OAuth Token:"), m_oauthEdit);
    
    auto *oauthHint = new QLabel(tr("<a href=\"https://twitchapps.com/tmi/\">Get OAuth Token</a>"), this);
    oauthHint->setOpenExternalLinks(true);
    connectionLayout->addRow(QString(), oauthHint);
    
    mainLayout->addWidget(connectionGroup);
    
    // Bot filter group
    auto *botGroup = new QGroupBox(tr("Bot Filter"), this);
    auto *botLayout = new QVBoxLayout(botGroup);
    
    auto *botLabel = new QLabel(tr("Bots to filter (one per line):"), this);
    botLayout->addWidget(botLabel);
    
    m_botListEdit = new QTextEdit(this);
    m_botListEdit->setPlaceholderText(tr("nightbot\nstreamElements\nmoobot\n..."));
    m_botListEdit->setMinimumHeight(100);
    botLayout->addWidget(m_botListEdit);
    
    mainLayout->addWidget(botGroup);
    
    // Buttons
    auto *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_okButton = new QPushButton(tr("OK"), this);
    m_cancelButton = new QPushButton(tr("Cancel"), this);
    
    buttonLayout->addWidget(m_okButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    connect(m_okButton, &QPushButton::clicked, this, &SettingsDialog::onAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void SettingsDialog::applyDarkTheme()
{
    setStyleSheet(QStringLiteral(
        "QDialog { background-color: #18181b; color: #efeff1; }"
        "QGroupBox { color: #efeff1; border: 1px solid #3d3d42; border-radius: 4px; margin-top: 8px; padding-top: 8px; }"
        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }"
        "QLineEdit, QTextEdit { background-color: #0e0e10; color: #efeff1; border: 1px solid #3d3d42; border-radius: 4px; padding: 4px; }"
        "QLineEdit:focus, QTextEdit:focus { border-color: #9147ff; }"
        "QLabel { color: #efeff1; }"
        "QLabel a { color: #9147ff; }"
        "QPushButton { background-color: #9147ff; color: #ffffff; border: none; border-radius: 4px; padding: 8px 16px; }"
        "QPushButton:hover { background-color: #772ce8; }"
        "QPushButton:pressed { background-color: #5c16c5; }"
    ));
}

void SettingsDialog::onAccept()
{
    accept();
}

QString SettingsDialog::channel() const
{
    return m_channelEdit->text().trimmed();
}

void SettingsDialog::setChannel(const QString &channel)
{
    m_channelEdit->setText(channel);
}

QString SettingsDialog::username() const
{
    return m_usernameEdit->text().trimmed();
}

void SettingsDialog::setUsername(const QString &username)
{
    m_usernameEdit->setText(username);
}

QString SettingsDialog::oauthToken() const
{
    QString token = m_oauthEdit->text().trimmed();
    // Remove "oauth:" prefix if user included it
    if (token.startsWith(QStringLiteral("oauth:"), Qt::CaseInsensitive)) {
        token = token.mid(6);
    }
    return token;
}

void SettingsDialog::setOauthToken(const QString &token)
{
    m_oauthEdit->setText(token);
}

QStringList SettingsDialog::botList() const
{
    QStringList bots;
    QString text = m_botListEdit->toPlainText();
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        QString trimmed = line.trimmed().toLower();
        if (!trimmed.isEmpty()) {
            bots.append(trimmed);
        }
    }
    return bots;
}

void SettingsDialog::setBotList(const QStringList &bots)
{
    m_botListEdit->setPlainText(bots.join('\n'));
}
