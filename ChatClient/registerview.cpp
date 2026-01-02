#include "registerview.h"
#include "ui_registerview.h"
#include <QMessageBox>
#include "idatabase.h"

RegisterView::RegisterView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::RegisterView)
{
    ui->setupUi(this);

    ui->btnRegister->setCursor(Qt::PointingHandCursor);//鼠标变成小手手
    ui->btnReturn->setCursor(Qt::PointingHandCursor);
}

RegisterView::~RegisterView()
{
    delete ui;
}

void RegisterView::on_btnReturn_clicked()//返回登录页面
{
    emit goLoginView();
}


void RegisterView::on_btnRegister_clicked()//点击注册按钮
{
    QString account = ui->accountEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    // 输入验证
    if(account.isEmpty() || password.isEmpty()){
        QMessageBox::warning(this, "输入错误", "账号和密码不能为空");
        return;
    }

    QString status = IDataBase::getInstance().userRegister(ui->accountEdit->text(),ui->passwordEdit->text());
    if(status=="registerOK"){
        QMessageBox::information(this, "注册成功", "请返回登录页面登录");
        emit goLoginView();
    }
    else if(status=="userExists") {
        QMessageBox::warning(this, "注册失败","用户名已存在");
    }
    else if(status=="registerFailed") {
        QMessageBox::critical(this, "注册失败","请稍后重试");
    }
    else {
        QMessageBox::critical(this, "错误", "未知错误状态：" + status);
    }
}

