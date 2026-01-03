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
    m_chatClient->connectToServer(QHostAddress("127.0.0.1"),1967);//我已经登录进来了直接连上就好了

    connect(m_chatClient,&ChatClient::connected,this,&MainWindow::connectedToServer);
    connect(m_chatClient,&ChatClient::jsonReceived,this,&MainWindow::jsonReceived);
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
            // 在主窗口也显示简要提示
            if (isForMe) {
                QString notification = QString("%1 给你发来私聊消息").arg(sender);
                //ui->statusBar->showMessage(notification, 3000);//等会换到通知框
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



