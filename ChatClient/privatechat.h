#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>
#include <QCloseEvent>
#include "chatclient.h"

namespace Ui {
class PrivateChat;
}

class PrivateChat : public QWidget
{
    Q_OBJECT

public:
    explicit PrivateChat(const QString &targetUser,const QString &myUsername,ChatClient *chatClient,QWidget *parent = nullptr);
    ~PrivateChat();

    QString getTargetUser() const { return m_targetUser; }

signals:
    void windowClosed(const QString &targetUser);

public slots:
    void displayPrivateMessage(const QString &sender,const QString &message,const QString &timestamp = "");

private slots:

    void on_btnSay_clicked();

private:
    Ui::PrivateChat *ui;
    QString m_targetUser;    // 聊天对象
    QString m_myUsername;    // 自己的用户名
    ChatClient *m_chatClient; // 共享的聊天客户端

    void sendPrivateMessage();
    void closeEvent(QCloseEvent *event) override;
};

#endif // PRIVATECHAT_H
