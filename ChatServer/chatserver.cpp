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
    connect(worker,&ServerWorker::disconnectedFromClient,this,std::bind(&ChatServer::userDisconnected,this,worker));
    m_clients.append(worker);//成功了就添加进来
    emit logMessage("新的用户连接上了");
}

void ChatServer::broadcast(const QJsonObject &message, ServerWorker *exclude)//给所有连接的客户端广播消息
{
    for(ServerWorker *worker : m_clients){
        worker->sendJson(message);
    }
}

void ChatServer::sendPrivateMessage(const QJsonObject &message, ServerWorker *sender)
{
    QString receiver = message.value("receiver").toString();
    QString senderName = message.value("sender").toString();
    // 查找接收者
    for(ServerWorker *worker : m_clients){
        if(worker->userName() == receiver){
            // 找到接收者，发送消息
            worker->sendJson(message);
            // 记录日志
            emit logMessage(QString("私聊: %1 -> %2").arg(senderName).arg(receiver));
            return;
        }
    }
    // 如果没有找到接收者，给发送者发送错误消息
    QJsonObject errorMessage;
    errorMessage["type"] = "private_error";
    errorMessage["text"] = QString("用户 %1 不在线").arg(receiver);
    errorMessage["receiver"] = receiver;
    sender->sendJson(errorMessage);
    emit logMessage(QString("私聊失败: 用户 %1 不存在或离线").arg(receiver));
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

        //还得把用户列表告诉新登进来的用户
        QJsonObject userListMessage;
        userListMessage["type"] = "userlist";
        QJsonArray userlist;
        for(ServerWorker *worker : m_clients){
            if(worker == sender) userlist.append(worker->userName()+ "*");
            else userlist.append(worker->userName());
        }
        userListMessage["userlist"] = userlist;
        sender->sendJson(userListMessage);
    }
    else if(typeVal.toString().compare("private", Qt::CaseInsensitive) == 0){
        // 处理私聊消息
        const QJsonValue textVal = docObj.value("text");
        const QJsonValue receiverVal = docObj.value("receiver");
        const QJsonValue senderVal = docObj.value("sender");
        if(textVal.isNull() || !textVal.isString()) return;
        if(receiverVal.isNull() || !receiverVal.isString()) return;
        QString text = textVal.toString().trimmed();
        QString receiver = receiverVal.toString().trimmed();
        if(text.isEmpty()) return;
        // 构建私聊消息
        QJsonObject privateMessage;
        privateMessage["type"] = "private";
        privateMessage["text"] = text;
        privateMessage["sender"] = sender->userName();
        privateMessage["receiver"] = receiver;
        privateMessage["timestamp"] = QDateTime::currentDateTime().toString("hh:mm:ss");

        // 发送私聊消息
        sendPrivateMessage(privateMessage, sender);
    }
}

void ChatServer::userDisconnected(ServerWorker *sender)
{
    m_clients.removeAll(sender);//移除数组中的这个客户端
    const QString userName = sender->userName();
    if(!userName.isEmpty()){
        QJsonObject disconnectedMessage;
        disconnectedMessage["type"]= "userdisconnected";
        disconnectedMessage["username"]= userName;
        broadcast(disconnectedMessage,nullptr);
        emit logMessage(userName + "disconnected");
    }
    sender->deleteLater();//真正删除掉
}
