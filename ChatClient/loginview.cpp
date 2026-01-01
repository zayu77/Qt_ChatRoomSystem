#include "loginview.h"
#include "ui_loginview.h"

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

void LoginView::on_btnRegister_clicked()
{
    emit goRegisterView();
}


void LoginView::on_btnLogin_clicked()
{

}

