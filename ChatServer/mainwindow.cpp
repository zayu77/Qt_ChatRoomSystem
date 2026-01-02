#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowFlag(Qt::FramelessWindowHint);//把原来的窗口界面隐藏
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btn_Close_clicked()
{
    this->close();
}

