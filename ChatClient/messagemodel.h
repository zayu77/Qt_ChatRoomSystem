#ifndef MESSAGEMODEL_H
#define MESSAGEMODEL_H

#include <QAbstractListModel>
#include <QDateTime>

struct ChatMessage {
    QString id;                // 消息ID
    QString sender;            // 发送者
    QString content;           // 内容
    QDateTime timestamp;       // 时间戳
    bool isMyMessage;          // 是否是我的消息
    bool isRead;               // 是否已读
    QString avatar;            // 头像
    int messageType;           // 0-文本 1-图片 2-文件
    QString filePath;          // 文件路径
    qint64 fileSize;          // 文件大小

    ChatMessage()
        : isMyMessage(false)
        , isRead(false)
        , messageType(0)
        , fileSize(0) {}
};

class MessageModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum MessageRoles {
        IdRole = Qt::UserRole + 1,
        SenderRole,
        ContentRole,
        TimestampRole,
        IsMyMessageRole,
        IsReadRole,
        AvatarRole,
        MessageTypeRole,
        FilePathRole,
        FileSizeRole
    };

    explicit MessageModel(QObject *parent = nullptr);

    // 重写基类方法
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    // 添加消息
    void addMessage(const ChatMessage &message);
    void addMessages(const QList<ChatMessage> &messages);

    // 更新消息状态
    void updateMessageStatus(const QString &id, bool isRead);

    // 清空消息
    void clear();

    // 获取消息
    ChatMessage getMessage(int index) const;
    QList<ChatMessage> getAllMessages() const;

    // 保存到数据库/从数据库加载
    bool saveToDatabase(int friendId);
    bool loadFromDatabase(int friendId, int limit = 100);

private:
    QList<ChatMessage> m_messages;
};
#endif // MESSAGEMODEL_H
