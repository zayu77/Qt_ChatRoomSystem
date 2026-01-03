#include "privatechat.h"
#include "ui_privatechat.h"
#include <QDateTime>
#include <QKeyEvent>
#include <QTextBlock>

PrivateChat::PrivateChat(const QString &targetUser,const QString &myUsername,ChatClient *chatClient,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrivateChat)
    , m_targetUser(targetUser)
    , m_myUsername(myUsername)
    , m_chatClient(chatClient)
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle(QString("与 %1 的私聊").arg(targetUser));
    ui->labelTitle->setText(QString(targetUser));//设置标题为对方用户名
    setAttribute(Qt::WA_DeleteOnClose);//关闭后销毁窗口

    // 显示欢迎消息
    QString welcomeMsg = QString("你正在与 %1 进行私聊").arg(targetUser);
    ui->Edit_communicate->append(QString("<div style='color: gray; font-style: italic;'>%1</div>").arg(welcomeMsg));
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

void PrivateChat::displayPrivateMessage(const QString &sender, const QString &message, const QString &timestamp)//展示私聊信息
{
    QString timeStr = timestamp.isEmpty() ?QDateTime::currentDateTime().toString("hh:mm:ss") :timestamp;

    QString formattedMsg;
    bool isFromMe = (sender == m_myUsername);

    if (isFromMe) {
        // 我发送的消息
        formattedMsg = QString("<div style='text-align: right; margin: 5px;'>""<div style='display: inline-block; max-width: 70%%; ""background-color: #DCF8C6; "
                               "border-radius: 10px; padding: 8px; ""text-align: left;'>""<div style='font-size: 11px; color: #888;'>%1</div>"
                               "<div style='font-size: 14px;'>%2</div>""</div>""</div>").arg(timeStr).arg(message.toHtmlEscaped());
    } else {
        // 对方发送的消息
        formattedMsg = QString("<div style='margin: 5px;'>""<div style='font-size: 12px; color: #666;'>%1</div>""<div style='display: inline-block; max-width: 70%%; "
                               "background-color: white; ""border: 1px solid #e1e1e1; ""border-radius: 10px; padding: 8px;'>"
                               "<div style='font-size: 14px;'>%2</div>""</div>""</div>").arg(timeStr).arg(message.toHtmlEscaped());
    }
    ui->Edit_communicate->append(formattedMsg);

    // 自动滚动到底部
    QTextCursor cursor = ui->Edit_communicate->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui->Edit_communicate->setTextCursor(cursor);
}

void PrivateChat::sendPrivateMessage()//发送私聊消息
{
    QString message = ui->say_textEdit->toPlainText().trimmed();
    if (message.isEmpty()) {
        return;
    }

    if (m_chatClient) {
        // 发送私聊消息
        m_chatClient->sendPrivateMessage(m_targetUser, message);

        // 在本地显示
        displayPrivateMessage(m_myUsername, message);

        // 清空输入框
        ui->say_textEdit->clear();
        ui->say_textEdit->setFocus();
    }
}

void PrivateChat::closeEvent(QCloseEvent *event)
{
    emit windowClosed(m_targetUser);
    event->accept();
}

void PrivateChat::on_btnSay_clicked()//发送消息
{
    sendPrivateMessage();
}

