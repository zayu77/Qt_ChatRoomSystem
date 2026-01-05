#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QJsonValue>
#include <QJsonObject>
#include <QMessageBox>

MainWindow::MainWindow(const QString &username,QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_userName=username;//保存好用户名
    m_chatClient = new ChatClient(this);//初始化一个客户端

    connect(m_chatClient,&ChatClient::connected,this,&MainWindow::connectedToServer);
    connect(m_chatClient,&ChatClient::jsonReceived,this,&MainWindow::jsonReceived);

    m_chatClient->connectToServer(QHostAddress("127.0.0.1"),1967);//我已经登录进来了直接连上就好了
    // 连接用户列表双击信号
    connect(ui->listWidget_users, &QListWidget::itemDoubleClicked,this, &MainWindow::on_listWidget_users_itemDoubleClicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectedToServer()
{
    m_chatClient->sendMessage(m_userName,"login");
    m_chatClient->setUserName(m_userName); //设置客户端用户名
}

void MainWindow::messageReceived(const QString &sender,const QString &text)
{
    ui->Edit_communicate->append(QString("%1 : %2").arg(sender).arg(text));
}

void MainWindow::jsonReceived(const QJsonObject &docObj)
{
    const QJsonValue typeVal= docObj.value("type");
    if(typeVal.isNull()||!typeVal.isString()) return ;
    if(typeVal.toString().compare("message",Qt::CaseInsensitive)==0){
        const QJsonValue textVal =docObj.value("text");
        const QJsonValue senderVal =docObj.value("sender");
        if(textVal.isNull()||!textVal.isString()) return ;
        if(senderVal.isNull()||!senderVal.isString()) return ;

        messageReceived(senderVal.toString(),textVal.toString());
    }
    else if(typeVal.toString().compare("newuser",Qt::CaseInsensitive)==0){
        const QJsonValue userNameVal =docObj.value("username");
        if(userNameVal.isNull()||!userNameVal.isString()) return ;
        userJoined(userNameVal.toString());//新用户就把他加到用户列表中
    }
    //处理其它客户端收到其它用户下线的逻辑
    else if(typeVal.toString().compare("userdisconnected",Qt::CaseInsensitive)==0){
        const QJsonValue userNameVal =docObj.value("username");
        if(userNameVal.isNull()||!userNameVal.isString()) return ;
        userLeft(userNameVal.toString());//在用户列表中移除掉下线的用户
    }
    //处理新登录进来的用户没有以前用户列表信息的逻辑
    else if(typeVal.toString().compare("userlist",Qt::CaseInsensitive)==0){
        const QJsonValue userlistVal =docObj.value("userlist");
        if(userlistVal.isNull()||!userlistVal.isArray()) return ;
        userListReceived(userlistVal.toVariant().toStringList());
    }
    else if(typeVal.toString().compare("private", Qt::CaseInsensitive) == 0){
        // 处理私聊消息
        const QJsonValue textVal = docObj.value("text");
        const QJsonValue senderVal = docObj.value("sender");
        const QJsonValue receiverVal = docObj.value("receiver");
        const QJsonValue timestampVal = docObj.value("timestamp");

        if(textVal.isNull() || !textVal.isString()) return;
        if(senderVal.isNull() || !senderVal.isString()) return;
        if(receiverVal.isNull() || !receiverVal.isString()) return;

        QString text = textVal.toString();
        QString sender = senderVal.toString();
        QString receiver = receiverVal.toString();
        QString timestamp = timestampVal.isString() ? timestampVal.toString() : "";

        // 判断是否是发给我的私聊
        bool isForMe = (receiver == m_userName);
        bool isFromMe = (sender == m_userName);

        if (isForMe || isFromMe) {
            // 确定私聊对象
            QString targetUser = isFromMe ? receiver : sender;

            // 如果对应的私聊窗口已打开，显示消息
            if (m_privateChatWindows.contains(targetUser)) {
                PrivateChat *window = m_privateChatWindows.value(targetUser);//从映射（Map）中根据用户名获取对应的私聊窗口对象
                window->displayPrivateMessage(sender, text, timestamp);
            } else {
                // 如果窗口未打开，先打开窗口再显示消息
                openPrivateChat(targetUser);
                if (m_privateChatWindows.contains(targetUser)) {
                    PrivateChat *window = m_privateChatWindows.value(targetUser);
                    window->displayPrivateMessage(sender, text, timestamp);
                }
            }
        }
    }
    else if(typeVal.toString().compare("private_error", Qt::CaseInsensitive) == 0){//发送的用户不在线就会出现私聊错误
        // 处理私聊错误
        const QJsonValue textVal = docObj.value("text");
        const QJsonValue receiverVal = docObj.value("receiver");

        if(!textVal.isNull() && textVal.isString()){
            QString errorMsg = textVal.toString();
            QString receiver = receiverVal.isString() ? receiverVal.toString() : "";

            QMessageBox::warning(this, "私聊发送失败",QString("给 %1 的消息发送失败: %2").arg(receiver).arg(errorMsg));
        }
    }
    else if(typeVal.toString().compare("friend_request", Qt::CaseInsensitive) == 0){
        // 处理收到的好友请求
        handleFriendRequest(docObj);
    }
    else if(typeVal.toString().compare("friend_added", Qt::CaseInsensitive) == 0){
        // 处理好友添加成功
        handleFriendAdded(docObj);
    }
    else if(typeVal.toString().compare("friend_error", Qt::CaseInsensitive) == 0){
        // 处理好友相关错误
        handleFriendError(docObj);
    }
    else if(typeVal.toString().compare("friend_list", Qt::CaseInsensitive) == 0){
        // 处理收到的好友列表
        handleFriendList(docObj);
    }
    else if(typeVal.toString().compare("friend_status", Qt::CaseInsensitive) == 0) {
        // 处理好友状态更新
        handleFriendStatus(docObj);
    }
    else if(typeVal.toString().compare("heartbeat_response", Qt::CaseInsensitive) == 0){
        qDebug() << "心跳正常";
        return;  // 直接返回，不干扰其他处理
    }
}

void MainWindow::userJoined(const QString &user)
{
    ui->listWidget_users->addItem(user);
}

void MainWindow::userLeft(const QString &user)//用户退出登录时处理的逻辑
{
    for(auto aItem: ui->listWidget_users->findItems(user,Qt::MatchExactly)){//退出登录后把用户删除
        ui->listWidget_users->removeItemWidget(aItem);
        delete aItem;
    }
}

void MainWindow::userListReceived(const QStringList &list)//给每个客户端的用户列表进行初始化
{
    ui->listWidget_users->clear();
    ui->listWidget_users->addItems(list);
    // 为每个用户项添加提示文本
    for(int i = 0; i < ui->listWidget_users->count(); ++i){
        QListWidgetItem *item = ui->listWidget_users->item(i);
        QString userName = item->text();

        if(userName.endsWith("*")){
            userName = userName.left(userName.length() - 1);
            if(userName == m_chatClient->userName()){
                item->setToolTip("这是你自己");
            } else {
                item->setToolTip("双击开始私聊");
            }
        } else {
            item->setToolTip("双击开始私聊");
        }
    }
}

void MainWindow::onPrivateChatWindowClosed(const QString &targetUser)//当用户关闭私聊窗口时，从管理器中清除对应记录
{
    if (m_privateChatWindows.contains(targetUser)) {
        m_privateChatWindows.remove(targetUser);
    }
}

void MainWindow::openPrivateChat(const QString &targetUser)//打开私聊窗口
{
    // 检查是否已打开与这个用户的私聊窗口
    if (m_privateChatWindows.contains(targetUser)) {
        // 如果已打开，激活窗口
        PrivateChat *window = m_privateChatWindows.value(targetUser);
        window->raise();//将窗口提升到最顶层
        window->activateWindow();//激活窗口，使其获得焦点
        return;
    }

    // 创建新的私聊窗口
    PrivateChat *privateWindow = new PrivateChat(targetUser, m_userName, m_chatClient, nullptr);

    // 连接关闭信号
    connect(privateWindow, &PrivateChat::windowClosed,this, &MainWindow::onPrivateChatWindowClosed);

    // 保存到映射中
    m_privateChatWindows.insert(targetUser, privateWindow);//就是保存到map中

    // 显示窗口
    privateWindow->show();
}

void MainWindow::handleFriendRequest(const QJsonObject &docObj)//处理好友请求
{
    QString fromUser = docObj.value("from").toString();
    QString nickname = docObj.value("nickname").toString("");
    QString timestamp = docObj.value("timestamp").toString("");

    QString message = QString("%1 请求添加你为好友").arg(fromUser);
    if (!nickname.isEmpty()) {
        message += QString("\n备注: %1").arg(nickname);
    }

    if (!timestamp.isEmpty()) {
        message += QString("\n时间: %1").arg(timestamp);
    }

    // 弹出确认对话框
    int ret = QMessageBox::question(this, "好友请求", message,QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

    // 发送响应
    QJsonObject response;
    response["type"] = "friend_response";
    response["to"] = fromUser;
    response["accepted"] = (ret == QMessageBox::Yes);
    response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    m_chatClient->sendJson(response);

    // 记录日志
    if (ret == QMessageBox::Yes) {
        qDebug() << "接受了" << fromUser << "的好友请求";
    } else {
        qDebug() << "拒绝了" << fromUser << "的好友请求";
    }
}

void MainWindow::handleFriendAdded(const QJsonObject &docObj)//好友已经添加成功，可以刷新好友列表了
{
    QString friendUsername = docObj.value("username").toString();
    QString nickname = docObj.value("nickname").toString("");
    QString timestamp = docObj.value("timestamp").toString("");

    QString displayName = nickname.isEmpty() ? friendUsername : nickname;
    QString message = QString("已成功添加 %1 为好友").arg(displayName);

    if (!timestamp.isEmpty()) {
        message += QString("\n时间: %1").arg(timestamp);
    }

    QMessageBox::information(this, "添加成功", message);

    // 可以刷新好友列表
    requestFriendList();
}

void MainWindow::requestFriendList()//请求刷新好友列表
{
    QJsonObject request;
    request["type"] = "get_friend_list";
    m_chatClient->sendJson(request);
    qDebug() << "请求好友列表...";
}

void MainWindow::handleFriendError(const QJsonObject &docObj)//处理添加好友失败的各种错误//这个可以说是补充
{
    QString code = docObj.value("code").toString("unknown");
    QString message = docObj.value("message").toString("");

    QString displayMsg = "添加好友失败: ";

    if (code == "user_not_found") {
        displayMsg += "用户不存在";
    } else if (code == "self_add") {
        displayMsg += "不能添加自己为好友";
    } else if (code == "already_friend") {
        displayMsg += "已经是好友";
    } else if (code == "user_offline") {
        displayMsg += "用户不在线";
    } else if (code == "request_expired") {
        displayMsg += "请求已过期";
    } else if (code == "add_failed") {
        displayMsg += "添加好友失败";
    } else {
        displayMsg += message.isEmpty() ? "未知错误" : message;
    }

    QMessageBox::warning(this, "添加失败", displayMsg);
}

void MainWindow::handleFriendList(const QJsonObject &docObj)//处理收到的好友列表
{
    QJsonArray friendsArray = docObj.value("friends").toArray();
    if (friendsArray.isEmpty()) {
        qDebug() << "好友列表为空";
        return;
    }
    qDebug() << "收到好友列表，数量:" << friendsArray.size();
    // 创建TreeWidget显示好友列表
    updateFriendTree(friendsArray);
}

void MainWindow::updateFriendTree(const QJsonArray &friends)
{
    // 先清空现有的好友列表
    ui->treeWidget_Friend->clear();

    // 按分组整理好友
    QMap<QString, QTreeWidgetItem*> groupItems;
    QMap<QString, int> groupCounts;

    for (const QJsonValue &value : friends) {
        QJsonObject friendObj = value.toObject();
        QString friendUsername = friendObj.value("friend_username").toString();
        QString nickname = friendObj.value("nickname").toString();
        QString groupName = friendObj.value("group_name").toString("我的好友");
        int status = friendObj.value("status").toInt(0);
        int groupId = friendObj.value("group_id").toInt(1);

        // 获取显示名称
        QString displayName = nickname.isEmpty() ? friendUsername : nickname;

        // 创建或获取分组
        if (!groupItems.contains(groupName)) {
            QTreeWidgetItem *groupItem = new QTreeWidgetItem(ui->treeWidget_Friend);
            groupItem->setText(0, groupName);
            groupItem->setData(0, Qt::UserRole, QString("group_%1").arg(groupId));

            // 设置分组图标
            groupItem->setIcon(0, QIcon(":/icons/folder.png"));

            groupItems[groupName] = groupItem;
            groupCounts[groupName] = 0;
        }

        // 创建好友项
        QTreeWidgetItem *friendItem = new QTreeWidgetItem(groupItems[groupName]);

        QString itemText = displayName;
        if (status == 1) {
            itemText += " [在线]";
            friendItem->setForeground(0, QColor(0, 128, 0));  // 绿色
            friendItem->setIcon(0, QIcon(":/icons/user-online.png"));
        } else {
            itemText += " [离线]";
            friendItem->setForeground(0, Qt::gray);
            friendItem->setIcon(0, QIcon(":/icons/user-offline.png"));
        }

        friendItem->setText(0, itemText);
        friendItem->setData(0, Qt::UserRole, friendUsername);
        friendItem->setData(0, Qt::UserRole + 1, displayName);  // 保存显示名称
        friendItem->setData(0, Qt::UserRole + 2, status);       // 保存状态

        // 设置工具提示
        QString tooltip = QString("用户名: %1").arg(friendUsername);
        if (!nickname.isEmpty()) {
            tooltip += QString("\n备注: %1").arg(nickname);
        }
        tooltip += QString("\n状态: %1").arg(status == 1 ? "在线" : "离线");
        tooltip += QString("\n分组: %1").arg(groupName);
        friendItem->setToolTip(0, tooltip);

        groupCounts[groupName]++;
    }

    // 更新分组标题显示数量
    for (auto it = groupItems.begin(); it != groupItems.end(); ++it) {
        QString groupName = it.key();
        int count = groupCounts[groupName];
        it.value()->setText(0, QString("%1 (%2)").arg(groupName).arg(count));
    }

    // 展开所有分组
    for (QTreeWidgetItem *item : groupItems) {
        item->setExpanded(true);
    }
}

void MainWindow::handleFriendStatus(const QJsonObject &docObj)
{
    QString username = docObj.value("username").toString();
    int status = docObj.value("status").toInt();

    qDebug() << "好友状态更新:" << username << "->" << (status == 1 ? "在线" : "离线");

    // 更新好友树中的状态
    updateFriendStatusInTree(username, status);

}

void MainWindow::updateFriendStatusInTree(const QString &username, int status)//更新好友的在线状态
{
    // 遍历所有分组
    for (int i = 0; i < ui->treeWidget_Friend->topLevelItemCount(); i++) {
        QTreeWidgetItem *groupItem = ui->treeWidget_Friend->topLevelItem(i);

        // 遍历分组中的所有好友
        for (int j = 0; j < groupItem->childCount(); j++) {
            QTreeWidgetItem *friendItem = groupItem->child(j);
            QString friendUsername = friendItem->data(0, Qt::UserRole).toString();

            if (friendUsername == username) {
                // 更新显示文本
                QString displayName = friendItem->data(0, Qt::UserRole + 1).toString();
                QString newText = displayName;

                if (status == 1) {
                    newText += " [在线]";
                    friendItem->setForeground(0, QColor(0, 128, 0));  // 绿色
                } else {
                    newText += " [离线]";
                    friendItem->setForeground(0, Qt::gray);
                }

                friendItem->setText(0, newText);
                friendItem->setData(0, Qt::UserRole + 2, status);

                // 更新工具提示
                QString tooltip = friendItem->toolTip(0);
                tooltip.replace(QRegularExpression("状态: .*"),QString("状态: %1").arg(status == 1 ? "在线" : "离线"));
                friendItem->setToolTip(0, tooltip);

                qDebug() << "更新好友树:" << username << "状态:" << status;
                return;
            }
        }
    }
}


void MainWindow::on_btnSay_clicked()//点击发送消息
{
    QString text = ui->say_textEdit->toPlainText().trimmed();
    if(text.isEmpty()) return;

    // 普通群聊消息
    m_chatClient->sendMessage(text);
    ui->say_textEdit->clear();//发完清空才是正确的
}


void MainWindow::on_btnLogout_clicked()
{
    m_chatClient->disconnectFromHost();//退出登录应该断开连接
    this->close();//应该需要关闭聊天室界面回到登录页面

    IDataBase::getInstance().userLogout(m_userName);
    // 创建新的 MasterView
    MasterView *newMasterView = new MasterView();
    newMasterView->show();
}


void MainWindow::on_listWidget_users_itemDoubleClicked(QListWidgetItem *item)//双击进入私聊窗口
{
    //选中某个用户，后面发信息相当于私发，别人收不到私发的消息
    QString userName = item->text();

    // 如果用户名以*结尾（表示自己），则不处理
    if(userName.endsWith("*")){
        return;
    }
    // 打开私聊聊天框
    openPrivateChat(userName);
}


void MainWindow::on_btnAddfriend_clicked()//添加好友
{
    AddFriendDialog dlg(this);
    dlg.setCurrentUser(m_userName);

    if (dlg.exec() == QDialog::Accepted) {
        QString targetUser = dlg.getTargetUsername();
        QString nickname = dlg.getNickname();
        int groupId = dlg.getGroupId();

        // 发送好友请求
        QJsonObject request;
        request["type"] = "friend_request";
        request["to"] = targetUser;
        if (!nickname.isEmpty()) {
            request["nickname"] = nickname;
        }
        request["group_id"] = groupId;

        m_chatClient->sendJson(request);

        QMessageBox::information(this, "已发送",QString("好友请求已发送给 %1").arg(targetUser));
    }
}

