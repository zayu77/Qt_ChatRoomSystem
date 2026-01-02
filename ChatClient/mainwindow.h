#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include "chatclient.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(const QString &username, QWidget *parent = nullptr);//登录的时候顺便把登录的用户名带过来
    ~MainWindow();

private slots:
    void connectedToServer();
    void messageReceived(const QString &sender,const QString &text);

    void on_btnSay_clicked();

    void on_btnLogout_clicked();

private:
    Ui::MainWindow *ui;

    ChatClient *m_chatClient;
    QString m_userName;
};
#endif // MAINWINDOW_H
