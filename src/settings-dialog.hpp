#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QPushButton>

class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog() override = default;

    QString channel() const;
    void setChannel(const QString &channel);

    QString username() const;
    void setUsername(const QString &username);

    QString oauthToken() const;
    void setOauthToken(const QString &token);

    QStringList botList() const;
    void setBotList(const QStringList &bots);

private slots:
    void onAccept();

private:
    void setupUi();
    void applyDarkTheme();

    QLineEdit *m_channelEdit;
    QLineEdit *m_usernameEdit;
    QLineEdit *m_oauthEdit;
    QTextEdit *m_botListEdit;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
};
