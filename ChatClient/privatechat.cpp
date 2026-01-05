#include "privatechat.h"
#include "ui_privatechat.h"
#include "messagedelegate.h"
#include "idatabase.h"
#include <QDateTime>
#include <QKeyEvent>
#include <QScrollBar>
#include <QListWidget>
#include <QFile>
#include <QUuid>

PrivateChat::PrivateChat(const QString &targetUser,const QString &myUsername,ChatClient *chatClient,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::PrivateChat)
    , m_targetUser(targetUser)
    , m_myUsername(myUsername)
    , m_chatClient(chatClient)
    , m_messageModel(new MessageModel(this))
{
    ui->setupUi(this);

    // 设置窗口属性
    setWindowTitle(QString("与 %1 的私聊").arg(targetUser));
    ui->labelTitle->setText(QString(targetUser));
    setAttribute(Qt::WA_DeleteOnClose);

    // 设置消息视图
    setupMessageView();

    // 加载聊天记录
    loadChatHistory();

    // 连接信号
    connect(ui->say_textEdit, &QTextEdit::textChanged, this, &PrivateChat::onInputTextChanged);

    // 设置输入框快捷键
    ui->say_textEdit->installEventFilter(this);

    // 设置输入框提示
    ui->say_textEdit->setPlaceholderText("输入消息...");

    // 初始禁用发送按钮
    ui->btnSay->setEnabled(false);

    setStyleSheet(
        "QLabel#labelTitle {"  // 标题标签样式
        "  color: purple;"
        "  padding: 10px;"
        "  border-bottom: 1px solid #e0e0e0;"
        "  font-size: 16px;"
        "  font-weight: bold;"
        "}"
        "QTextEdit#say_textEdit {"
        "  border: 1px solid #ddd;"
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  font-size: 13px;"
        "  background-color: white;"
        "}"
        "QTextEdit#say_textEdit:focus {"
        "  border: 1px solid #4d90fe;"
        "}"
        ""
        "QPushButton#btnSay {"
        "  background-color: #07c160;"
        "  color: white;"
        "  border: none;"
        "  border-radius: 4px;"
        "  padding: 8px 20px;"
        "  font-size: 13px;"
        "  font-weight: 500;"
        "}"
        "QPushButton#btnSay:hover {"
        "  background-color: #06ad56;"
        "}"
        "QPushButton#btnSay:pressed {"
        "  background-color: #059b4e;"
        "}"
        "QPushButton#btnSay:disabled {"
        "  background-color: #cccccc;"
        "  color: #666666;"
        "}"
        );
}

PrivateChat::~PrivateChat()
{
    delete ui;
}

void PrivateChat::setupMessageView()
{
    QListView *messageListView = ui->messageListView;

    if (!messageListView) {
        qWarning() << "messageListView is null!";
        return;
    }

    // 设置模型
    messageListView->setModel(m_messageModel);

    // 设置委托
    MessageDelegate *delegate = new MessageDelegate(this);
    messageListView->setItemDelegate(delegate);

    // 设置视图属性
    messageListView->setSelectionMode(QAbstractItemView::NoSelection);
    messageListView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    messageListView->setSpacing(0);//项间距
    messageListView->setWordWrap(true);
    messageListView->setUniformItemSizes(false);
    messageListView->setResizeMode(QListView::Adjust);

    // 设置样式
    messageListView->setStyleSheet(
        "QListView {"
        "  background-color: #f5f5f5;"
        "  border: none;"
        "  outline: none;"
        "}"
        "QListView::item {"
        "  border: none;"
        "  background-color: transparent;"
        "}"
    );

    // 自动滚动到底部
    connect(m_messageModel, &QAbstractItemModel::rowsInserted,this, [messageListView](const QModelIndex &parent, int first, int last) {
                Q_UNUSED(parent)
                Q_UNUSED(first)
                Q_UNUSED(last)
                messageListView->scrollToBottom();});

    // 连接到数据变化信号
    connect(m_messageModel, &MessageModel::dataChanged,this, [messageListView]() {
                messageListView->viewport()->update();});
}

void PrivateChat::displayPrivateMessage(const QString &sender,const QString &message,const QString &timestamp)
{
    // 创建消息对象
    ChatMessage msg;
    msg.id = QUuid::createUuid().toString();
    msg.sender = sender;
    msg.content = message;
    msg.timestamp = timestamp.isEmpty() ?QDateTime::currentDateTime() :QDateTime::fromString(timestamp, "hh:mm:ss");
    msg.isMyMessage = (sender == m_myUsername);
    msg.isRead = true;  // 接收到时默认已读
    msg.messageType = 0;  // 文本消息

    // 添加到模型
    m_messageModel->addMessage(msg);

    // 保存到数据库
    QString conversationId = m_targetUser;  // 使用对方用户名作为会话ID

    IDataBase::getInstance().saveChatMessage(
        conversationId,
        msg.id,
        sender,
        m_targetUser,
        message,
        0,  // messageType
        "", // filePath
        0,  // fileSize
        msg.isMyMessage,
        true,  // isRead
        msg.timestamp
        );
}

void PrivateChat::sendPrivateMessage()
{
    QString message = ui->say_textEdit->toPlainText().trimmed();
    if (message.isEmpty()) {
        return;
    }

    if (m_chatClient) {
        // 生成消息ID
        QString messageId = QUuid::createUuid().toString();

        // 先在本地显示
        ChatMessage localMsg;
        localMsg.id = messageId;
        localMsg.sender = m_myUsername;
        localMsg.content = message;
        localMsg.timestamp = QDateTime::currentDateTime();
        localMsg.isMyMessage = true;
        localMsg.isRead = false;  // 刚发送，还未被对方读取
        localMsg.messageType = 0;

        m_messageModel->addMessage(localMsg);

        // 保存到本地数据库
        QString conversationId = m_targetUser;
        IDataBase::getInstance().saveChatMessage(
            conversationId,
            messageId,
            m_myUsername,
            m_targetUser,
            message,
            0,  // messageType
            "", // filePath
            0,  // fileSize
            true,  // isMyMessage
            false, // isRead
            localMsg.timestamp
            );

        // 发送到服务器
        m_chatClient->sendPrivateMessage(m_targetUser, message);

        // 清空输入框
        ui->say_textEdit->clear();
        ui->say_textEdit->setFocus();

        // 禁用发送按钮
        ui->btnSay->setEnabled(false);
    }
}

void PrivateChat::loadChatHistory()//加载聊天记录
{
    QString conversationId = m_targetUser;
    QList<QJsonObject> history = IDataBase::getInstance().getChatHistory(conversationId, 50);

    // 反转列表，因为数据库中是倒序
    std::reverse(history.begin(), history.end());

    for (const QJsonObject &msg : history) {
        ChatMessage chatMsg;
        chatMsg.id = msg["id"].toString();
        chatMsg.sender = msg["sender"].toString();
        chatMsg.content = msg["content"].toString();
        chatMsg.timestamp = QDateTime::fromString(msg["timestamp"].toString(), "yyyy-MM-dd hh:mm:ss");
        chatMsg.isMyMessage = msg["isMyMessage"].toBool();
        chatMsg.isRead = msg["isRead"].toBool();
        chatMsg.messageType = msg["messageType"].toInt();
        chatMsg.filePath = msg["filePath"].toString();
        chatMsg.fileSize = msg["fileSize"].toInt();

        m_messageModel->addMessage(chatMsg);
    }

    qDebug() << "加载聊天记录：" << m_targetUser << "，数量：" << history.size();
}

bool PrivateChat::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->say_textEdit && event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);

        // Ctrl+Enter 发送
        if (keyEvent->key() == Qt::Key_Return &&
            (keyEvent->modifiers() & Qt::ControlModifier)) {
            sendPrivateMessage();
            return true;
        }
        // Shift+Enter 换行
        else if (keyEvent->key() == Qt::Key_Return &&
                 (keyEvent->modifiers() & Qt::ShiftModifier)) {
            // 插入换行
            ui->say_textEdit->insertPlainText("\n");
            return true;
        }
    }

    return QWidget::eventFilter(obj, event);
}

void PrivateChat::closeEvent(QCloseEvent *event)
{
    emit windowClosed(m_targetUser);
    event->accept();
}

void PrivateChat::on_btnSay_clicked()
{
    sendPrivateMessage();
}

void PrivateChat::onInputTextChanged()
{
    QString text = ui->say_textEdit->toPlainText().trimmed();
    ui->btnSay->setEnabled(!text.isEmpty());
}
