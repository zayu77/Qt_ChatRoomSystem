#include "chatserver.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>

ChatServer::ChatServer(QObject *parent):
    QTcpServer(parent)
{

}

void ChatServer::incomingConnection(qintptr socketDescriptor)//这个有新客户端连接时会自动调用
{
    ServerWorker *worker= new ServerWorker(this);
    if(!worker->setSocketDescriptor(socketDescriptor)){
        worker->deleteLater();
        return;
    }
    connect(worker,&ServerWorker::logMessage,this,&ChatServer::logMessage);//这个logMessage的传递由ServerWorker传到ChatServer再到mainwindow
    connect(worker,&ServerWorker::jsonReceived,this,&ChatServer::jsonReceived);//接收到ServerWorker发出的信号然后调用ChatServer的方法
    // connect(worker,&ServerWorker::disconnectedFromClient,this,std::bind(&ChatServer::userDisconnected,this,worker));
    m_clients.append(worker);//成功了就添加进来
    emit logMessage("新的用户连接上了");
}

void ChatServer::broadcast(const QJsonObject &message, ServerWorker *exclude)//给所有连接的客户端广播消息
{
    for(ServerWorker *worker : m_clients){
        worker->sendJson(message);
    }
}

void ChatServer::stopServer()//你是？？？好像没用到
{
    close();
}

void ChatServer::jsonReceived(ServerWorker *sender, const QJsonObject &docObj)
{
    const QJsonValue typeVal= docObj.value("type");
    if(typeVal.isNull()||!typeVal.isString()) return ;
    if(typeVal.toString().compare("message",Qt::CaseInsensitive)==0){
        const QJsonValue textVal =docObj.value("text");
        if(textVal.isNull()||!textVal.isString()) return ;
        const QString text= textVal.toString().trimmed();
        if(text.isEmpty()) return ;
        QJsonObject message;
        message["type"] = "message";
        message["text"] = text;
        message["sender"] = sender->userName();
        broadcast(message,sender);
    }
    else if(typeVal.toString().compare("login",Qt::CaseInsensitive)==0){
        const QJsonValue userNameVal =docObj.value("text");
        if(userNameVal.isNull()||!userNameVal.isString()) return ;
        sender->setUserName(userNameVal.toString());//把登录的用户名传过来设置好，后面发消息才能获取到用户名
        QJsonObject connectedMessage;
        connectedMessage["type"] = "newuser";
        connectedMessage["username"] = userNameVal.toString();
        broadcast(connectedMessage,sender);
    }
}
