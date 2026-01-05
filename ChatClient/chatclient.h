#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>

class ChatClient : public QObject
{
    Q_OBJECT
public:
    explicit ChatClient(QObject *parent = nullptr);
    void sendPrivateMessage(const QString &receiver, const QString &text);
    // 发送通用 JSON
    void sendJson(const QJsonObject &json);
    void setUserName(const QString &name) { m_userName = name; }
    QString userName() const { return m_userName; }
    // 添加一个简单的方法//心跳
    void startHeartbeat();

signals:
    void connected();
    void messageReceived(const QString &text);
    void jsonReceived(const QJsonObject &docObj);
    //心跳新增
    void disconnected();

private:
    QTcpSocket *m_clientSocket;
    QString m_userName;

    //心跳机制
    QTimer *m_heartbeatTimer;    // 心跳定时器
    QTimer *m_reconnectTimer;    // 重连定时器
    QString m_serverHost;        // 服务器地址
    quint16 m_serverPort;        // 服务器端口
    bool m_shouldReconnect;      // 是否应该重连

public slots:
    void onReadyRead();
    void sendMessage(const QString &text,const QString &type = "message");
    void connectToServer(const QHostAddress &address,quint16 port);
    void disconnectFromHost();

    //心跳机制
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);
    void sendHeartbeat();
    void tryReconnect();
};

#endif // CHATCLIENT_H
