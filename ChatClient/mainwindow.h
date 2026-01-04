#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include "chatclient.h"
#include <masterview.h>
#include "privatechat.h"
#include "idatabase.h"
#include "addfrienddialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &username, QWidget *parent = nullptr);//登录的时候顺便把登录的用户名带过来
    ~MainWindow();

private slots:
    void connectedToServer();
    void messageReceived(const QString &sender,const QString &text);
    void jsonReceived(const QJsonObject &docObj);
    void userJoined(const QString &user);
    void userLeft(const QString &user);
    void userListReceived(const QStringList &list);

    // 新增槽函数
    void onPrivateChatWindowClosed(const QString &targetUser);
    void openPrivateChat(const QString &targetUser);

    void on_btnSay_clicked();

    void on_btnLogout_clicked();

    void on_listWidget_users_itemDoubleClicked(QListWidgetItem *item);

    void on_btnAddfriend_clicked();

private:
    Ui::MainWindow *ui;

    // 新增：管理私聊窗口
    QMap<QString, PrivateChat*> m_privateChatWindows;
    ChatClient *m_chatClient;
    QString m_userName;
};
#endif // MAINWINDOW_H
