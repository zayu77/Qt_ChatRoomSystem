#include "chatserver.h"

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
    // connect(worker,&ServerWorker::logMessage,this,&ChatServer::logMessage);
    // connect(worker,&ServerWorker::jsonReceived,this,&ChatServer::jsonReceived);
    // connect(worker,&ServerWorker::disconnectedFromClient,this,std::bind(&ChatServer::userDisconnected,this,worker));
    m_clients.append(worker);//成功了就添加进来
    emit logMessage("新的用户连接上了");
}

void ChatServer::stopServer()//你是？？？
{
    close();
}
