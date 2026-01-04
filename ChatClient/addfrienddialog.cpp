#include "addfrienddialog.h"
#include "ui_addfrienddialog.h"
#include <QMessageBox>

AddFriendDialog::AddFriendDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AddFriendDialog),
    m_groupId(1)  // 默认分组ID
{
    ui->setupUi(this);

    // 设置对话框属性
    setWindowTitle("添加好友");
    setFixedSize(300, 200);

    // 连接信号
    connect(ui->btnYes, &QPushButton::clicked, this, &AddFriendDialog::on_btnYes_clicked);
    connect(ui->btnCancel, &QPushButton::clicked, this, &AddFriendDialog::reject);
}

AddFriendDialog::~AddFriendDialog()
{
    delete ui;
}

void AddFriendDialog::setCurrentUser(const QString &username)//设置使用者是谁
{
    m_currentUser = username;
}

QString AddFriendDialog::getTargetUsername() const
{
    return m_targetUsername;
}

QString AddFriendDialog::getNickname() const
{
    return m_nickname;
}

int AddFriendDialog::getGroupId() const
{
    return m_groupId;
}

void AddFriendDialog::on_btnYes_clicked()
{
    QString username = ui->userNameEdit->text().trimmed();
    QString nickname = ui->nickNameEdit->text().trimmed();
    int groupId = ui->GroupcomboBox->currentData().toInt();

    if (username.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名");
        return;
    }

    // 不能添加自己
    if (username == m_currentUser) {
        QMessageBox::warning(this, "错误", "不能添加自己为好友");
        return;
    }

    m_targetUsername = username;
    m_nickname = nickname;
    m_groupId = groupId;

    accept();
}

