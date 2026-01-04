#ifndef ADDFRIENDDIALOG_H
#define ADDFRIENDDIALOG_H

#include <QDialog>

namespace Ui {
class AddFriendDialog;
}

class AddFriendDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddFriendDialog(QWidget *parent = nullptr);
    ~AddFriendDialog();

    // 设置当前用户（用于防止添加自己）
    void setCurrentUser(const QString &username);
    // 获取输入的数据
    QString getTargetUsername() const;
    QString getNickname() const;
    int getGroupId() const;

private slots:
    void on_btnYes_clicked();

private:
    Ui::AddFriendDialog *ui;

    QString m_currentUser;  // 当前登录用户的用户名
    QString m_targetUsername; //想要添加的好友
    QString m_nickname;
    int m_groupId;
};

#endif // ADDFRIENDDIALOG_H
