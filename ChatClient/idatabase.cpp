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
