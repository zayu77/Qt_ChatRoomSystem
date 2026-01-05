#include "chatclient.h"
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>

//每个人都有自己的底层客户端
ChatClient::ChatClient(QObject *parent) : QObject{ parent }
    , m_clientSocket(new QTcpSocket(this))
    , m_heartbeatTimer(new QTimer(this))
    , m_reconnectTimer(new QTimer(this))
    , m_shouldReconnect(true)  // 默认开启重连
{
    m_clientSocket=new QTcpSocket(this);
    connect(m_clientSocket,&QTcpSocket::connected,this,&ChatClient::connected);//连接成功时触发
    connect(m_clientSocket,&QTcpSocket::readyRead,this,&ChatClient::onReadyRead);//有数据时触发

    // 新增：监听socket状态变化
    connect(m_clientSocket, &QTcpSocket::stateChanged,this, &ChatClient::onSocketStateChanged);

    // 心跳定时器：每30秒发送一次心跳
    m_heartbeatTimer->setInterval(30000);
    connect(m_heartbeatTimer, &QTimer::timeout, this, &ChatClient::sendHeartbeat);

    // 重连定时器：每5秒尝试重连一次
    m_reconnectTimer->setInterval(5000);
    connect(m_reconnectTimer, &QTimer::timeout, this, &ChatClient::tryReconnect);
}

void ChatClient::sendPrivateMessage(const QString &receiver, const QString &text)//构造私聊消息结构
{
    if(m_clientSocket->state() != QAbstractSocket::ConnectedState) return;
    if(receiver.isEmpty() || text.isEmpty()) return;

    QDataStream serverStream(m_clientSocket);
    serverStream.setVersion(QDataStream::Qt_6_5);

    QJsonObject message;
    message["type"] = "private";
    message["receiver"] = receiver;
    message["text"] = text;
    //message["sender"] = m_userName;  // 需要先保存用户名
    serverStream << QJsonDocument(message).toJson();
}

void ChatClient::sendJson(const QJsonObject &json)//直接发送json格式的信息
{
    if (m_clientSocket->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "无法发送：未连接到服务器";
        return;
    }

    QDataStream serverStream(m_clientSocket);
    serverStream.setVersion(QDataStream::Qt_6_5);

    QByteArray jsonData = QJsonDocument(json).toJson(QJsonDocument::Compact);

    qDebug() << "发送JSON:" << QString::fromUtf8(jsonData);
    serverStream << jsonData;
}

void ChatClient::onReadyRead()// 接收服务器消息
{
    QByteArray jsonData;
    QDataStream socketStream(m_clientSocket);
    socketStream.setVersion(QDataStream::Qt_6_5);
    for(;;){
        socketStream.startTransaction(); // 开始事务，可以回滚
        socketStream>>jsonData;
        if(socketStream.commitTransaction()){ // 如果成功读取完整数据包
            QJsonParseError parseError;
            const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData,&parseError);// 将字节数组解析为JSON文档
            if(parseError.error == QJsonParseError::NoError){
                if(jsonDoc.isObject()){
                    emit jsonReceived(jsonDoc.object());
                }
            }
        }
        else {
            break;
        }
    }
}

void ChatClient::sendMessage(const QString &text, const QString &type)//向服务器发送消息
{
    // 检查连接状态
    if(m_clientSocket->state()!=QAbstractSocket::ConnectedState) return;
    if(!text.isEmpty()){
        QDataStream serverStream(m_clientSocket);
        serverStream.setVersion(QDataStream::Qt_6_5);
        QJsonObject message;
        message["type"]=type;
        message["text"]=text;
        serverStream<<QJsonDocument(message).toJson(); //转换为字节流并发送
    }
}

void ChatClient::connectToServer(const QHostAddress &address, quint16 port)
{
    m_clientSocket->connectToHost(address,port);//根据地址和端口连接服务器
}

void ChatClient::disconnectFromHost()//断开连接
{
    m_clientSocket->disconnectFromHost();
}

void ChatClient::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    qDebug() << "Socket state changed:" << socketState;

    if (socketState == QAbstractSocket::ConnectedState) {
        // 连接成功，开始心跳
        m_heartbeatTimer->start();
        m_reconnectTimer->stop();  // 停止重连
        qDebug() << "Connected, heartbeat started";

    } else if (socketState == QAbstractSocket::UnconnectedState) {
        // 连接断开，停止心跳
        m_heartbeatTimer->stop();

        // 如果需要重连，启动重连定时器
        if (m_shouldReconnect) {
            m_reconnectTimer->start();
            qDebug() << "Disconnected, reconnect timer started";
        }

        emit disconnected();
    }
}

void ChatClient::sendHeartbeat()
{
    if (m_clientSocket->state() != QAbstractSocket::ConnectedState) {
        return;
    }

    QJsonObject heartbeat;
    heartbeat["type"] = "heartbeat";
    heartbeat["timestamp"] = QDateTime::currentDateTime().toString();

    sendJson(heartbeat);
    qDebug() << "Heartbeat sent";
}

void ChatClient::tryReconnect()
{
    if (m_clientSocket->state() == QAbstractSocket::ConnectedState) {
        m_reconnectTimer->stop();
        return;
    }

    qDebug() << "Trying to reconnect to server...";
    m_clientSocket->connectToHost(QHostAddress(m_serverHost), m_serverPort);
}


