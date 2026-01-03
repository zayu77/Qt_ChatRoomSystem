#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(const QString &username,QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_userName=username;//保存好用户名
    m_chatClient = new ChatClient(this);//初始化一个客户端
    m_chatClient->connectToServer(QHostAddress("127.0.0.1"),1967);//我已经登录进来了直接连上就好了

    connect(m_chatClient,&ChatClient::connected,this,&MainWindow::connectedToServer);
    connect(m_chatClient,&ChatClient::messageReceived,this,&MainWindow::messageReceived);
    //connect(m_chatClient,&ChatClient::jsonReceived,this,&MainWindow::jsonReceived);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectedToServer()
{
    m_chatClient->sendMessage(m_userName,"login");
}

void MainWindow::messageReceived(const QString &text)
{
    ui->Edit_communicate->append(text);
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
    this->close();
}

