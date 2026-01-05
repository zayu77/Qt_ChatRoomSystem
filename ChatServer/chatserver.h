#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QObject>
#include <QTcpServer>
#include "serverworker.h"
#include "idatabase.h"

class ChatServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit ChatServer(QObject *parent = nullptr);

protected:
    void incomingConnection(qintptr socketDescriptor) override;
    QVector<ServerWorker*>m_clients;

    void broadcast(const QJsonObject &message,ServerWorker *exclude);
    void sendFriendList(ServerWorker *client, const QString &username);//发送好友列表
    // 通知好友状态变化
    void notifyFriendsStatusChange(const QString &username, int status);
    ServerWorker* findWorkerByUsername(const QString &username);

    //多线程
    void onConnectionReady(qintptr socketDescriptor);
    void onConnectionFailed(qintptr socketDescriptor, const QString &error);
    void setupWorkerConnections(ServerWorker *worker);

private:
    void sendPrivateMessage(const QJsonObject &message, ServerWorker *sender);
    QMap<QString, QMap<QString, QJsonObject>> m_pendingRequests;  // 待处理的好友请求

signals:
    void logMessage(const QString &msg);

public slots:
    void stopServer();
    void jsonReceived(ServerWorker *sender,const QJsonObject &docObj);
    void userDisconnected(ServerWorker *sender);
};

#endif // CHATSERVER_H
