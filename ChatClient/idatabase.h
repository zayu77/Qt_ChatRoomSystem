#ifndef IDATABASE_H
#define IDATABASE_H

#include <QObject>
#include <QtSql>
#include <QSqlDatabase>

class IDataBase : public QObject
{
    Q_OBJECT
public:
    //单例模式
    static IDataBase& getInstance()
    {
        static IDataBase instance;
        return instance;
    }

    QString userRegister(QString userName,QString password);//注册
    QString userLogin(QString userName,QString password);//登录
    void userLogout(const QString &userName);//退出登录

    // 新增好友相关方法
    bool addFriendRelationship(int userId, int friendId, int groupId = 1, const QString &nickname = "");

    // 获取好友列表
    QJsonArray getFriendList(int userId);

    // 获取用户ID
    int getUserIdByUsername(const QString &username);

    // 聊天记录相关方法
    bool saveChatMessage(const QString &conversationId,
                         const QString &messageId,
                         const QString &sender,
                         const QString &receiver,
                         const QString &content,
                         int messageType = 0,
                         const QString &filePath = "",
                         qint64 fileSize = 0,
                         bool isMyMessage = false,
                         bool isRead = false,
                         const QDateTime &timestamp = QDateTime());

    QList<QJsonObject> getChatHistory(const QString &conversationId,int limit = 100,int offset = 0);

private:
    explicit IDataBase(QObject *parent = nullptr);
    IDataBase(IDataBase const&) = delete;
    void operator=(IDataBase const&)  = delete;

    QSqlDatabase database;

    void initDatabase();//初始化数据库

    QString m_userName;

signals:

};

#endif // IDATABASE_H
