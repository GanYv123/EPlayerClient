#include "widget.h"
#include "loginform.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;

    w.hide();
    LoginForm login;
    login.show();
    login.connect(&login,&LoginForm::login,&w,&Widget::show);

    qDebug() << __FILE__ << "(" << __LINE__ << "):";
    int ret = a.exec();
    qDebug() << __FILE__ << "(" << __LINE__ << "):";
    return ret;
}
