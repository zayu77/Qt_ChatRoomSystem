#include "idatabase.h"

QString IDataBase::userRegister(QString userName, QString password)//注册处理逻辑
{
    // 检查用户名是否已存在
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT username FROM user WHERE username = :USER");
    checkQuery.bindValue(":USER", userName);
    checkQuery.exec();

    if(checkQuery.first()){
        qDebug() << "Username already exists:" << userName;
        return "userExists";
    }

    // 生成用户ID（简单实现：时间戳）
    QString userId = QString::number(QDateTime::currentSecsSinceEpoch());

    // 插入新用户
    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO user (id, username, password, status) "
                        "VALUES (:ID, :USER, :PWD, 0)");
    insertQuery.bindValue(":ID", userId);
    insertQuery.bindValue(":USER", userName);
    insertQuery.bindValue(":PWD", password);

    if(insertQuery.exec()){
        qDebug() << "Register successful for user:" << userName;
        return "registerOK";
    } else {
        qDebug() << "Register failed for user:" << userName;
        return "registerFailed";
    }
}

QString IDataBase::userLogin(QString userName, QString password)//登录处理逻辑
{
    QSqlQuery query;
    query.prepare("select username,password from user where username = :USER");//预处理语句，:USER是占位符由后面的变量替换
    query.bindValue(":USER",userName);
    query.exec();
    if(query.first()&&query.value("username").isValid()){
        QString pwd=query.value("password").toString();
        if(pwd==password) {
            qDebug()<<"login OK";
            // 更新状态为在线
            QSqlQuery updateQuery;
            updateQuery.prepare("UPDATE User SET STATUS = 1 WHERE USERNAME = :USER");
            updateQuery.bindValue(":USER", userName);
            updateQuery.exec();
            m_userName=userName;//登录的时候保存下来
            return "loginOK";
        }
        else {
            qDebug()<<"wrong password";
            return "wrongPassword";
        }
    }
    else {
        qDebug()<<"no such user"<<userName;
        return "wrongUserName";
    }
}

void IDataBase::userLogout(const QString &userName)
{
    if (userName.isEmpty()) {
        qDebug() << "Logout failed: username is empty";
        return;
    }

    QSqlQuery query;
    query.prepare("UPDATE user SET status = 0 WHERE username = :USER");
    query.bindValue(":USER", userName);

    if(query.exec()) {
        qDebug() << "User logout successful, status updated to offline:" << userName;
    } else {
        qDebug() << "Failed to update user status:" << query.lastError().text();
    }
}

bool IDataBase::addFriendRelationship(int userId, int friendId, int groupId, const QString &nickname)//添加好友
{
    // 检查是否已经是好友
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT id FROM friend_relationships WHERE user_id = :UID AND friend_id = :FID");
    checkQuery.bindValue(":UID", userId);
    checkQuery.bindValue(":FID", friendId);
    if (checkQuery.exec() && checkQuery.first()) {
        qDebug() << "Already friends";
        return false;
    }

    // 添加好友关系
    QSqlQuery query;
    query.prepare("INSERT INTO friend_relationships (user_id, friend_id, group_id, nickname) "
                  "VALUES (:UID, :FID, :GID, :NICK)");
    query.bindValue(":UID", userId);
    query.bindValue(":FID", friendId);
    query.bindValue(":GID", groupId);
    query.bindValue(":NICK", nickname);

    bool success = query.exec();
    if (success) {
        qDebug() << "Friend relationship added: user" << userId << "-> friend" << friendId;
    } else {
        qDebug() << "Failed to add friend relationship:" << query.lastError();
    }

    return success;
}

bool IDataBase::isFriend(int userId, int friendId)//是否已经是好友
{
    if (userId <= 0 || friendId <= 0) return false;

    QSqlQuery query;
    query.prepare(
        "SELECT id FROM friend_relationships "
        "WHERE user_id = :USER_ID AND friend_id = :FRIEND_ID"
        );
    query.bindValue(":USER_ID", userId);
    query.bindValue(":FRIEND_ID", friendId);

    if (!query.exec()) {
        qDebug() << "检查好友关系失败:" << query.lastError();
        return false;
    }

    return query.first();  // 如果有记录，说明是好友
}

QJsonArray IDataBase::getFriendList(int userId)//获取好友列表
{
    QJsonArray friendList;

    QSqlQuery query;
    query.prepare(
        "SELECT "
        "u.username as friend_username, "
        "u.status as online_status, "
        "fr.nickname as remark_name, "
        "fg.name as group_name, "
        "fg.id as group_id "
        "FROM friend_relationships fr "
        "JOIN user u ON fr.friend_id = u.id "
        "LEFT JOIN friend_groups fg ON fr.group_id = fg.id "
        "WHERE fr.user_id = :UID "
        "ORDER BY fg.sort_order, u.status DESC, fr.nickname, u.username"
        );
    query.bindValue(":UID", userId);

    if (!query.exec()) {
        qDebug() << "Failed to get friend list:" << query.lastError();
        return friendList;
    }

    while (query.next()) {
        QJsonObject friendObj;
        friendObj["friend_username"] = query.value("friend_username").toString();
        friendObj["status"] = query.value("online_status").toInt();

        QString nickname = query.value("remark_name").toString();
        if (!nickname.isEmpty()) {
            friendObj["nickname"] = nickname;
        }

        friendObj["group_name"] = query.value("group_name").toString();
        friendObj["group_id"] = query.value("group_id").toInt();

        friendList.append(friendObj);
    }

    qDebug() << "Found" << friendList.size() << "friends for user" << userId;
    return friendList;
}

int IDataBase::getUserIdByUsername(const QString &username)//根据用户名获取到用户id
{
    QSqlQuery query;
    query.prepare("SELECT id FROM user WHERE username = :USER");
    query.bindValue(":USER", username);

    if (query.exec() && query.first()) {
        return query.value("id").toInt();
    }
    return -1;  // 用户不存在
}

IDataBase::IDataBase(QObject *parent) : QObject{parent}
{
    initDatabase();
}

void IDataBase::initDatabase()//初始化数据库
{
    database=QSqlDatabase::addDatabase("QSQLITE");
    QString aFile="E:\\qtC++\\samples\\LAB_SQLlite\\LAB4.db";
    database.setDatabaseName(aFile);

    //不管你的路径是否正确，都会返回打开
    if(!database.open()){
        qDebug()<<"failed to open database";
    }else {
        qDebug()<<"open database is ok";
    }
}
