#include "serverworker.h"
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>

//处理客户端请求
ServerWorker::ServerWorker(QObject *parent): QObject{ parent }
{
    m_serverSocket = new QTcpSocket(this);
    connect(m_serverSocket,&QTcpSocket::readyRead,this,&ServerWorker::onReadyRead);
    //connect(m_serverSocket,&QTcpSocket::disconnected,this,&ServerWorker::disconnectedFromClient);
}

bool ServerWorker::setSocketDescriptor(qintptr socketDescriptor)//设置客户端的 socket
{
    return m_serverSocket->setSocketDescriptor(socketDescriptor);
}

void ServerWorker::onReadyRead()//当客户端有数据可读时自动触发，读取并处理收到的 JSON 数据
{
    QByteArray jsonData;
    QDataStream socketStream(m_serverSocket);// 创建数据流对象，关联到客户端的socket
    socketStream.setVersion(QDataStream::Qt_6_5);
    for(;;){
        socketStream.startTransaction(); // 开始事务，可以回滚
        socketStream>>jsonData;
        if(socketStream.commitTransaction()){ // 如果成功读取完整数据包
            emit logMessage(QString::fromUtf8(jsonData));
            sendMessage("I recieved message");
            // QJsonParseError parseError;
            // const QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonData,&parseError);// 将字节数组解析为JSON文档
            // if(parseError.error == QJsonParseError::NoError){
            //     if(jsonDoc.isObject()){
            //         emit logMessage(QJsonDocument(jsonDoc).toJson(QJsonDocument::Compact));
            //         emit jsonReceived(this,jsonDoc.object());
            //     }
            // }
        }
        else {
            break;
        }
    }
}

void ServerWorker::sendMessage(const QString &text, const QString &type)//向客户端发送结构化的 JSON 消息
{
    // 检查连接状态
    if(m_serverSocket->state()!=QAbstractSocket::ConnectedState) return;
    if(!text.isEmpty()){
        QDataStream serverStream(m_serverSocket);
        serverStream.setVersion(QDataStream::Qt_6_5);
        QJsonObject message;
        message["type"]=type;
        message["text"]=text;
        serverStream<<QJsonDocument(message).toJson(); //转换为字节流并发送
    }
}
