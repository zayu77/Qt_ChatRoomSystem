#ifndef PRIVATECHAT_H
#define PRIVATECHAT_H

#include <QWidget>
#include <QCloseEvent>
#include "chatclient.h"
#include "messagemodel.h"

namespace Ui {
class PrivateChat;
}

class PrivateChat : public QWidget
{
    Q_OBJECT

public:
    explicit PrivateChat(const QString &targetUser,
                         const QString &myUsername,
                         ChatClient *chatClient,
                         QWidget *parent = nullptr);
    ~PrivateChat();

    QString getTargetUser() const { return m_targetUser; }

    // 添加消息
    void displayPrivateMessage(const QString &sender,
                               const QString &message,
                               const QString &timestamp = "");

signals:
    void windowClosed(const QString &targetUser);

private slots:
    void on_btnSay_clicked();
    void onInputTextChanged();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::PrivateChat *ui;
    QString m_targetUser;       // 聊天对象
    QString m_myUsername;       // 自己的用户名
    ChatClient *m_chatClient;   // 共享的聊天客户端
    MessageModel *m_messageModel; // 消息模型

    void setupMessageView();    // 设置消息视图
    void sendPrivateMessage();  // 发送私聊消息
    void loadChatHistory();     // 加载聊天记录

    // 输入处理
    bool eventFilter(QObject *obj, QEvent *event) override;

};

#endif // PRIVATECHAT_H
