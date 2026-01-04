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
    bool removeFriendRelationship(int userId, int friendId);
    bool updateFriendGroup(int userId, int friendId, int newGroupId);
    //bool updateFriendNickname(int userId, int friendId, const QString &nickname);

    // 获取好友列表
    QJsonArray getFriendList(int userId);

    // 获取用户ID
    int getUserIdByUsername(const QString &username);

    // 更新用户状态
    bool updateUserStatus(const QString &username, int status);

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
