#ifndef REGISTERVIEW_H
#define REGISTERVIEW_H

#include <QWidget>

namespace Ui {
class RegisterView;
}

class RegisterView : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterView(QWidget *parent = nullptr);
    ~RegisterView();

private slots:
    void on_btnReturn_clicked();

    void on_btnRegister_clicked();

signals:
    void goLoginView();

private:
    Ui::RegisterView *ui;
};

#endif // REGISTERVIEW_H
