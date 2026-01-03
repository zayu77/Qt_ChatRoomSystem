#include "loginview.h"
#include "ui_loginview.h"
#include <QMessageBox>
#include "idatabase.h"
#include "mainwindow.h"

LoginView::LoginView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LoginView)
{
    ui->setupUi(this);

    //设置一下按钮的hover
    ui->btnLogin->setCursor(Qt::PointingHandCursor);
    ui->btnRegister->setCursor(Qt::PointingHandCursor);
}

LoginView::~LoginView()
{
    delete ui;
}

void LoginView::on_btnRegister_clicked()//转到注册页面
{
    emit goRegisterView();
}


void LoginView::on_btnLogin_clicked()//点击登录按钮
{
    QString account = ui->accountEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    // 输入验证
    if(account.isEmpty() || password.isEmpty()){
        QMessageBox::warning(this, "输入错误", "账号和密码不能为空");
        return;
    }

    QString status = IDataBase::getInstance().userLogin(ui->accountEdit->text(),ui->passwordEdit->text());
    if(status=="loginOK"){
        QMessageBox::information(this, "登录成功", "开始你的聊天之旅吧!");
        jumpToChatRoom(account);//切换到聊天室
        return;
    }
    else if(status=="wrongUserName") {
        QMessageBox::warning(this, "登录失败","用户名不存在");
        return;
    }
    else if(status=="wrongPassword") {
        QMessageBox::warning(this, "登录失败","密码错误");
        return;
    }
    else {
        QMessageBox::critical(this, "错误", "未知错误状态：" + status);
        return;
    }
}

void LoginView::jumpToChatRoom(const QString &username)//切换到聊天室
{

    MainWindow *chatWindow = new MainWindow(username);
    chatWindow->show();

    // 发射信号
    emit loginSuccessAndJump();

    //关闭登录窗口
    this->close();
}

