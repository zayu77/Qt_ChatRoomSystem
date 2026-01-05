#include "messagemodel.h"
#include <QDebug>

MessageModel::MessageModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int MessageModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_messages.size();
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_messages.size())
        return QVariant();

    const ChatMessage &msg = m_messages.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return msg.content;
    case IdRole:
        return msg.id;
    case SenderRole:
        return msg.sender;
    case ContentRole:
        return msg.content;
    case TimestampRole:
        return msg.timestamp;
    case IsMyMessageRole:
        return msg.isMyMessage;
    case IsReadRole:
        return msg.isRead;
    case AvatarRole:
        return msg.avatar;
    case MessageTypeRole:
        return msg.messageType;
    case FilePathRole:
        return msg.filePath;
    case FileSizeRole:
        return msg.fileSize;
    default:
        return QVariant();
    }
}

QHash<int, QByteArray> MessageModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IdRole] = "messageId";
    roles[SenderRole] = "sender";
    roles[ContentRole] = "content";
    roles[TimestampRole] = "timestamp";
    roles[IsMyMessageRole] = "isMyMessage";
    roles[IsReadRole] = "isRead";
    roles[AvatarRole] = "avatar";
    roles[MessageTypeRole] = "messageType";
    roles[FilePathRole] = "filePath";
    roles[FileSizeRole] = "fileSize";
    return roles;
}

void MessageModel::addMessage(const ChatMessage &message)
{
    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size());
    m_messages.append(message);
    endInsertRows();

    // 发送数据变化信号
    QModelIndex newIndex = index(m_messages.size() - 1);
    emit dataChanged(newIndex, newIndex);
}

void MessageModel::addMessages(const QList<ChatMessage> &messages)
{
    if (messages.isEmpty()) return;

    beginInsertRows(QModelIndex(), m_messages.size(), m_messages.size() + messages.size() - 1);
    m_messages.append(messages);
    endInsertRows();
}

void MessageModel::clear()
{
    if (m_messages.isEmpty()) return;

    beginRemoveRows(QModelIndex(), 0, m_messages.size() - 1);
    m_messages.clear();
    endRemoveRows();
}
