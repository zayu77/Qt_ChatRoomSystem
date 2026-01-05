#include "chatserver.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include "connectiontask.h"

ChatServer::ChatServer(QObject *parent):
    QTcpServer(parent)
{
    // 配置线程池
    // QThreadPool *pool = QThreadPool::globalInstance();
    // pool->setMaxThreadCount(20);  // 最大20个线程
    // pool->setExpiryTimeout(30000);  // 线程空闲30秒后回收

    // qDebug() << "ChatServer created, thread pool size:" << pool->maxThreadCount();
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

    // qDebug() << "New incoming connection, socket:" << socketDescriptor;
    // qDebug() << "Server thread:" << QThread::currentThread();

    // // 使用线程池验证socket
    // ConnectionTask *task = new ConnectionTask(socketDescriptor, this);

    // // 连接信号
    // connect(task, &ConnectionTask::connectionReady,this, &ChatServer::onConnectionReady, Qt::QueuedConnection);
    // connect(task, &ConnectionTask::connectionFailed,this, &ChatServer::onConnectionFailed, Qt::QueuedConnection);

    // // 在线程池中执行验证
    // QThreadPool::globalInstance()->start(task);

    // emit logMessage(QString("新连接已分配给线程池处理 (socket: %1)").arg(socketDescriptor));
}

void ChatServer::broadcast(const QJsonObject &message, ServerWorker *exclude)//给所有连接的客户端广播消息
{
    for(ServerWorker *worker : m_clients){
        worker->sendJson(message);
    }
}

void ChatServer::sendFriendList(ServerWorker *client, const QString &username)//根据你是谁给你发你的好友列表
{
    int userId = IDataBase::getInstance().getUserIdByUsername(username);
    if (userId == -1) return;

    QJsonArray friendList = IDataBase::getInstance().getFriendList(userId);

    QJsonObject response;
    response["type"] = "friend_list";
    response["friends"] = friendList;

    client->sendJson(response);
}

void ChatServer::notifyFriendsStatusChange(const QString &username, int status)//通知好友上线和下线
{
    qDebug() << "通知" << username << "的状态变化:" << (status == 1 ? "上线" : "下线");

    int userId = IDataBase::getInstance().getUserIdByUsername(username);
    if (userId == -1) return;

    // 获取该用户的所有好友
    QJsonArray friendList = IDataBase::getInstance().getFriendList(userId);

    for (const QJsonValue &value : friendList) {
        QJsonObject friendObj = value.toObject();
        QString friendUsername = friendObj["friend_username"].toString();

        // 查找好友是否在线
        ServerWorker *friendWorker = findWorkerByUsername(friendUsername);
        if (friendWorker) {
            QJsonObject statusMsg;
            statusMsg["type"] = "friend_status";
            statusMsg["username"] = username;
            statusMsg["status"] = status;  // 1-上线 0-下线
            statusMsg["timestamp"] = QDateTime::currentDateTime().toString();

            friendWorker->sendJson(statusMsg);
            qDebug() << "通知" << friendUsername << "：" << username << (status == 1 ? "上线" : "下线");
        }
    }
}

ServerWorker *ChatServer::findWorkerByUsername(const QString &username)//根据用户名找对应的worker
{
    if (username.isEmpty()) {
        return nullptr;
    }
    for (ServerWorker *worker : m_clients) {
        if (worker->userName() == username) {
            return worker;
        }
    }
    return nullptr;  // 没找到
}

void ChatServer::onConnectionReady(qintptr socketDescriptor)
{
    qDebug() << "[ChatServer] Creating worker for socket:" << socketDescriptor;

    // 在主线程中创建worker
    ServerWorker *worker = new ServerWorker(this);

    if (!worker->setSocketDescriptor(socketDescriptor)) {
        qDebug() << "[ChatServer] Failed to set socket descriptor";
        emit logMessage("创建客户端处理器失败");
        worker->deleteLater();
        return;
    }

    // 保存到客户端列表
    m_clients.append(worker);

    // 设置信号连接
    setupWorkerConnections(worker);

    qDebug() << "[ChatServer] Worker created successfully";
    emit logMessage("客户端连接已建立");
}

void ChatServer::onConnectionFailed(qintptr socketDescriptor, const QString &error)
{
    qDebug() << "[ChatServer] Socket failed:" << socketDescriptor << "error:" << error;
    emit logMessage(QString("连接验证失败: %1").arg(error));
}

void ChatServer::setupWorkerConnections(ServerWorker *worker)
{
    qDebug() << "[ChatServer] Setting up worker connections";
    qDebug() << "[ChatServer] Worker thread:" << worker->thread();

    // 消息接收
    connect(worker, &ServerWorker::jsonReceived,this, &ChatServer::jsonReceived, Qt::QueuedConnection);

    // 客户端断开连接
    connect(worker, &ServerWorker::disconnectedFromClient,
            this, [this, worker]() {
                userDisconnected(worker);
            }, Qt::QueuedConnection);

    // 日志消息
    connect(worker, &ServerWorker::logMessage,this, &ChatServer::logMessage, Qt::QueuedConnection);
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
        //发送好友列表
        sendFriendList(sender, userNameVal.toString());
        notifyFriendsStatusChange(userNameVal.toString(), 1);//告诉好友们自己上线了
    }
    else if(typeVal.toString().compare("private", Qt::CaseInsensitive) == 0){
        // 处理私聊消息
        const QJsonValue textVal = docObj.value("text");
        const QJsonValue receiverVal = docObj.value("receiver");
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
    else if (typeVal.toString().compare("friend_request", Qt::CaseInsensitive) == 0) {
        // 处理好友请求
        QString fromUser = sender->userName();
        QString toUser = docObj.value("to").toString();
        QString nickname = docObj.value("nickname").toString("");
        int groupId = docObj.value("group_id").toInt(1);

        int fromId = IDataBase::getInstance().getUserIdByUsername(fromUser);
        int toId = IDataBase::getInstance().getUserIdByUsername(toUser);
        if (toId == -1) {
            // 目标用户不存在
            QJsonObject error;
            error["type"] = "friend_error";//这个类型我的客户端可没处理
            error["code"] = "user_not_found";
            error["message"] = "用户不存在";
            sender->sendJson(error);
            return;
        }
        if (fromId == toId) {
            // 不能添加自己
            QJsonObject error;
            error["type"] = "friend_error";
            error["code"] = "self_add";
            error["message"] = "不能添加自己为好友";
            sender->sendJson(error);
            return;
        }
        // 检查是否已经是好友
        if (IDataBase::getInstance().isFriend(fromId, toId)) {
            QJsonObject error;
            error["type"] = "friend_error";
            error["code"] = "already_friend";
            error["message"] = "已经是好友";
            sender->sendJson(error);
            return;
        }
        // 查找目标用户是否在线
        ServerWorker *targetWorker = nullptr;
        for (ServerWorker *worker : m_clients) {
            if (worker->userName() == toUser) {
                targetWorker = worker;
                break;
            }
        }

        if (targetWorker) {
            // 用户在线，转发请求
            QJsonObject forwardRequest = docObj;
            forwardRequest["type"] = "friend_request";
            forwardRequest["from"] = fromUser;
            forwardRequest["timestamp"] = QDateTime::currentDateTime().toString();
            targetWorker->sendJson(forwardRequest);

            // 记录待处理的请求
            m_pendingRequests[toUser][fromUser] = QJsonObject({
                {"from", fromUser},
                {"to", toUser},
                {"nickname", nickname},
                {"group_id", groupId},
                {"timestamp", QDateTime::currentDateTime().toString()}
            });

            emit logMessage(QString("好友请求: %1 -> %2").arg(fromUser).arg(toUser));

        } else {
            // 用户不在线
            QJsonObject error;
            error["type"] = "friend_error";
            error["code"] = "user_offline";
            error["message"] = "用户不在线";
            sender->sendJson(error);
        }
    }
    else if (typeVal.toString().compare("friend_response", Qt::CaseInsensitive) == 0) {
        // 处理好友请求响应（应该叫friend_response而不是friend_accept）
        QString responder = sender->userName();  // 响应者
        QString requester = docObj.value("to").toString();  // 请求者
        bool accepted = docObj.value("accepted").toBool();

        int responderId = IDataBase::getInstance().getUserIdByUsername(responder);
        int requesterId = IDataBase::getInstance().getUserIdByUsername(requester);

        if (responderId == -1 || requesterId == -1) return;

        if (accepted) {
            // 检查是否有待处理的请求
            if (!m_pendingRequests.contains(responder) ||
                !m_pendingRequests[responder].contains(requester)) {
                qDebug() << "没有找到对应的好友请求";
                return;
            }

            QJsonObject originalRequest = m_pendingRequests[responder][requester];
            int groupId = originalRequest.value("group_id").toInt(1);
            QString nickname = originalRequest.value("nickname").toString();

            // 建立双向好友关系
            bool success1 = IDataBase::getInstance().addFriendRelationship(responderId, requesterId, groupId, "");
            bool success2 = IDataBase::getInstance().addFriendRelationship(requesterId, responderId, groupId, nickname);

            if (success1 && success2) {
                // 通知请求方
                ServerWorker *requesterWorker = findWorkerByUsername(requester);
                if (requesterWorker) {
                    QJsonObject successMsg;
                    successMsg["type"] = "friend_added";
                    successMsg["username"] = responder;
                    successMsg["nickname"] = nickname;
                    successMsg["group_id"] = groupId;
                    requesterWorker->sendJson(successMsg);

                    // 发送更新后的好友列表
                    sendFriendList(requesterWorker, requester);
                }

                // 通知响应方
                QJsonObject successMsg2;
                successMsg2["type"] = "friend_added";
                successMsg2["username"] = requester;
                sender->sendJson(successMsg2);

                // 发送更新后的好友列表
                sendFriendList(sender, responder);
                if (m_pendingRequests.contains(requester) &&
                    m_pendingRequests[requester].contains(responder)) {
                    m_pendingRequests[requester].remove(responder);
                }
                emit logMessage(QString("好友关系建立: %1 <-> %2").arg(requester).arg(responder));
            }
        }
        // 清理待处理请求
        m_pendingRequests[responder].remove(requester);
    }
    else if(typeVal.toString().compare("get_friend_list", Qt::CaseInsensitive) == 0){
        // 处理获取好友列表请求
        QString username = sender->userName();
        if (!username.isEmpty()) {
            sendFriendList(sender, username);
        }
    }
    else  if (typeVal.toString().compare("heartbeat", Qt::CaseInsensitive) == 0) { // 处理心跳
        qDebug() << "Heartbeat received from:" << sender->userName();

        // 简单回复
        QJsonObject response;
        response["type"] = "heartbeat_response";
        response["timestamp"] = QDateTime::currentDateTime().toString();
        sender->sendJson(response);
        return;
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
        notifyFriendsStatusChange(userName, 0);//告诉好友们我下线了
        emit logMessage(userName + "disconnected");
    }
    sender->deleteLater();//真正删除掉
}
