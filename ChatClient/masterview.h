#ifndef MASTERVIEW_H
#define MASTERVIEW_H

#include <QWidget>
#include <QEvent>
#include <QMouseEvent>
#include <loginview.h>
#include <registerview.h>

QT_BEGIN_NAMESPACE
namespace Ui {
class MasterView;
}
QT_END_NAMESPACE

class MasterView : public QWidget
{
    Q_OBJECT

public:
    MasterView(QWidget *parent = nullptr);
    ~MasterView();

public slots:
    void goLoginView();
    void goRegisterView();

private slots:
    void on_btnClose_clicked();

private:
    Ui::MasterView *ui;

    void pushWidgetToStackView(QWidget *widget);

    LoginView *loginView;
    RegisterView *registerView;
};

#endif // MASTERVIEW_H
