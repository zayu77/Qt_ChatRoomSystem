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
    //connect(m_chatClient,&ChatClient::messageReceived,this,&MainWindow::messageReceived);
    connect(m_chatClient,&ChatClient::jsonReceived,this,&MainWindow::jsonReceived);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectedToServer()
{
    m_chatClient->sendMessage(m_userName,"login");
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
}

