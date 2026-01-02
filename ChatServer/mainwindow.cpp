#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlag(Qt::FramelessWindowHint);//把原来的窗口界面隐藏
    m_chatServer=new ChatServer(this);

    connect(m_chatServer,&ChatServer::logMessage,this,&MainWindow::logMessage);
    ui->startStopButton->setText("启动服务器");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_Close_clicked()//隐藏了窗口，新加了一个关闭按钮
{
    this->close();
}


void MainWindow::on_startStopButton_clicked()//点击启动服务器或者关闭服务器
{
    if(m_chatServer->isListening()){ // 如果服务器正在监听（运行中）
        m_chatServer->stopServer();
        ui->startStopButton->setText("启动服务器");
        logMessage("服务器已停止");
    }
    else{
        if(!m_chatServer->listen(QHostAddress::Any,1967)){ //尝试启动服务器，监听所有网络接口
            QMessageBox::critical(this,"错误","无法启动服务器");
            return;
        }
        logMessage("服务器已启动");
        ui->startStopButton->setText("停止服务器");
    }
}

void MainWindow::logMessage(const QString &msg)//发送日志信息到日志框中
{
    ui->logEditer->appendPlainText(msg);
}

