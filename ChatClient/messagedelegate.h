// messagedelegate.h
#ifndef MESSAGEDELEGATE_H
#define MESSAGEDELEGATE_H

#include <QStyledItemDelegate>

class MessageDelegate : public QStyledItemDelegate
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

    explicit MessageDelegate(QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option,const QModelIndex &index) const override;
    void paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const override;

private:
    // 计算消息气泡大小
    QSize calculateBubbleSize(const QString &text, const QFont &font,int maxWidth = 300) const;

    // 绘制消息气泡
    void drawMessageBubble(QPainter *painter, const QRect &rect,bool isMyMessage, bool isSelected) const;

    // 绘制文本
    void drawMessageText(QPainter *painter, const QRect &rect,const QString &text, const QFont &font) const;

    // 绘制时间
    void drawMessageTime(QPainter *painter, const QRect &rect,const QString &time, bool isMyMessage) const;

    // 绘制发送者头像
    void drawAvatar(QPainter *painter, const QRect &rect,const QString &avatarPath, const QString &sender) const;
};
#endif // MESSAGEDELEGATE_H
