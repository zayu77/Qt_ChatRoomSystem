#include "masterview.h"
#include "ui_masterview.h"

MasterView::MasterView(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::MasterView)
{
    ui->setupUi(this);

    setWindowFlag(Qt::FramelessWindowHint);//把原来的窗口界面隐藏
    goLoginView();//初始在登录界面
}

MasterView::~MasterView()
{
    delete ui;
}

void MasterView::goLoginView()
{
    loginView = new LoginView(this);
    pushWidgetToStackView(loginView);
    connect(loginView,SIGNAL(loginSuccess()),this,SLOT(goWelcomeView()));
    connect(loginView,SIGNAL(goRegisterView()),this,SLOT(goRegisterView()));//转到注册页面
}

void MasterView::goRegisterView()
{
    registerView = new RegisterView(this);
    pushWidgetToStackView(registerView);
    connect(registerView,SIGNAL(goLoginView()),this,SLOT(goLoginView()));//转到登录页面
}

void MasterView::pushWidgetToStackView(QWidget *widget)
{
    ui->stackedWidget->addWidget(widget);
    int count=ui->stackedWidget->count();
    ui->stackedWidget->setCurrentIndex(count-1);
    //ui->label_Title->setText(widget->windowTitle());//界面的名字
}

void MasterView::on_btnClose_clicked()
{
    this->close();
}

