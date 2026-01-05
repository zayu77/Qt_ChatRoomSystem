// connectiontask.cpp
#include "connectiontask.h"
#include <QTcpSocket>
#include <QThread>
#include <QDebug>

ConnectionTask::ConnectionTask(qintptr socketDescriptor, QObject *parent)
    : QObject(parent)
    , m_socketDescriptor(socketDescriptor)
{
    qDebug() << "[ConnectionTask] Created for socket:" << socketDescriptor;
}

ConnectionTask::~ConnectionTask()
{
    qDebug() << "[ConnectionTask] Destroyed";
}

void ConnectionTask::run()
{
    qDebug() << "[ConnectionTask] Running in thread:" << QThread::currentThread();

    // 只做socket有效性验证
    QTcpSocket testSocket;
    bool isValid = testSocket.setSocketDescriptor(m_socketDescriptor);

    if (isValid) {
        qDebug() << "[ConnectionTask] Socket" << m_socketDescriptor << "is valid";
        emit connectionReady(m_socketDescriptor);
    } else {
        QString error = testSocket.errorString();
        qDebug() << "[ConnectionTask] Socket" << m_socketDescriptor << "invalid:" << error;
        emit connectionFailed(m_socketDescriptor, error);
    }
}
