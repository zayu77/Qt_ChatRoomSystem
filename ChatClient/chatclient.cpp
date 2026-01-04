#include "chatclient.h"
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>

//每个人都有自己的底层客户端
ChatClient::ChatClient(QObject *parent) : QObject{ parent }
{
    m_clientSocket=new QTcpSocket(this);
    connect(m_clientSocket,&QTcpSocket::connected,this,&ChatClient::connected);//连接成功时触发
    connect(m_clientSocket,&QTcpSocket::readyRead,this,&ChatClient::onReadyRead);//有数据时触发
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


