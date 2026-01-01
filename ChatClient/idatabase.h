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

private:
    explicit IDataBase(QObject *parent = nullptr);
    IDataBase(IDataBase const&) = delete;
    void operator=(IDataBase const&)  = delete;

    QSqlDatabase database;

    void initDatabase();//初始化数据库

signals:

};

#endif // IDATABASE_H
