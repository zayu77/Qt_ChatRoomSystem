// connectiontask.h
#ifndef CONNECTIONTASK_H
#define CONNECTIONTASK_H

#include <QObject>
#include <QRunnable>

class ConnectionTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit ConnectionTask(qintptr socketDescriptor, QObject *parent = nullptr);
    ~ConnectionTask();

    void run() override;

signals:
    void connectionReady(qintptr socketDescriptor);
    void connectionFailed(qintptr socketDescriptor, const QString &error);

private:
    qintptr m_socketDescriptor;
};
#endif // CONNECTIONTASK_H
