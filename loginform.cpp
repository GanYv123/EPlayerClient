#include "loginform.h"
#include "ui_loginform.h"
#include "widget.h"
#include <time.h>
#include <QPixmap>
#include <QMessageBox>
#include <QRandomGenerator>
#include <QCryptographicHash>
//y%dcunyd6x202lmfm9=-y7#bd-w(ro4*(9u$0i-3#$txwbkzg$
const char* MD5_KEY = "*&^%$#@b.v+h-b*g/h@n!h#n$d^ssx,.kl<kl";
//const char* HOST = "http://192.168.0.152:8000";
//const char* HOST = "http://code.edoyun.com:9530";
const char* HOST = "http://192.168.133.133:9999";
bool LOGIN_STATUS = false;

LoginForm::LoginForm(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::LoginForm),
    escCount(0),
    auto_login_id(-1) // 初始化定时器 ID，-1 表示无效状态
{
    // 创建保存记录的文件对象，用于存储用户登录信息
    record = new RecordFile("edoyun.dat");

    // 设置 UI
    ui->setupUi(this);

    // 设置窗口无边框样式
    this->setWindowFlag(Qt::FramelessWindowHint);

    // 设置用户名输入框的占位文本，去掉边框，并安装事件过滤器
    ui->nameEdit->setPlaceholderText(u8"用户名/手机号/邮箱");
    ui->nameEdit->setFrame(false);
    ui->nameEdit->installEventFilter(this);

    // 设置密码输入框的占位文本、去掉边框、设置为密码模式，并安装事件过滤器
    ui->pwdEdit->setPlaceholderText(u8"填写密码");
    ui->pwdEdit->setFrame(false);
    ui->pwdEdit->setEchoMode(QLineEdit::Password);
    ui->pwdEdit->installEventFilter(this);

    // 为“忘记密码”按钮安装事件过滤器
    ui->forget->installEventFilter(this);

    // 初始化网络访问管理器，用于处理登录请求
    net = new QNetworkAccessManager(this);
    // 连接网络请求完成信号到槽函数
    connect(net, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(slots_login_request_finshed(QNetworkReply*)));

    // 配置 InfoForm 子窗口的样式和行为
    info.setWindowFlag(Qt::FramelessWindowHint);         // 设置为无边框窗口
    info.setWindowModality(Qt::ApplicationModal);        // 设置为模态窗口，阻止其他窗口操作

    // 加载配置文件，初始化登录设置
    load_config();
}


LoginForm::~LoginForm()
{
    //qDebug() << __FILE__ << "(" << __LINE__ << "):";
    delete record;
    delete ui;
    delete net;
    //qDebug() << __FILE__ << "(" << __LINE__ << "):";
}

bool LoginForm::eventFilter(QObject* watched, QEvent* event)
{
    if(ui->pwdEdit == watched)
    {
        if(event->type() == QEvent::FocusIn)
        {
            ui->pwdEdit->setStyleSheet("color: rgb(251, 251, 251);background-color: transparent;");
        }
        else if(event->type() == QEvent::FocusOut)
        {
            if(ui->pwdEdit->text().size() == 0)
            {
                ui->pwdEdit->setStyleSheet("color: rgb(71, 75, 94);background-color: transparent;");
            }
        }
    }
    else if(ui->nameEdit == watched)
    {
        if(event->type() == QEvent::FocusIn)
        {
            ui->nameEdit->setStyleSheet("color: rgb(251, 251, 251);background-color: transparent;");
        }
        else if(event->type() == QEvent::FocusOut)
        {
            if(ui->nameEdit->text().size() == 0)
            {
                ui->nameEdit->setStyleSheet("color: rgb(71, 75, 94);background-color: transparent;");
            }
        }
    }
    if((ui->forget == watched) && (event->type() == QEvent::MouseButtonPress))
    {
        //qDebug() << __FILE__ << "(" << __LINE__ << "):";
        QDesktopServices::openUrl(QUrl(QString(HOST) + "/forget"));
    }
    return QWidget::eventFilter(watched, event);
}

void LoginForm::timerEvent(QTimerEvent* event)
{
    if(event->timerId() == auto_login_id)
    {
        killTimer(auto_login_id);
        QJsonObject& root = record->config();
        QString user = root["user"].toString();
        QString pwd = root["password"].toString();
        check_login(user, pwd);
    }
}

/**
 * @brief LoginForm::on_logoButton_released
 * 登陆按钮点击
 */

void LoginForm::on_logoButton_released()
{
    if(ui->logoButton->text() == u8"取消自动登录")
    {
        killTimer(auto_login_id);
        auto_login_id = -1;
        ui->logoButton->setText(u8"登录");
    }
    else
    {
        QString user = ui->nameEdit->text();
        //检查用户名的有效性
        if(user.size() == 0 || user == u8"用户名/手机号/邮箱")
        {
            info.set_text(u8"用户不能为空\r\n请输入用户名", u8"确认").show();
            ui->nameEdit->setFocus();
            return;
        }
        //检查密码的有效性
        QString pwd = ui->pwdEdit->text();
        if(pwd.size() == 0 || pwd == u8"填写密码")
        {
            info.set_text(u8"密码不能为空\r\n请输入密码", u8"确认").show();
            ui->pwdEdit->setFocus();
            return;
        }
        check_login(user, pwd);
    }
}

void LoginForm::on_remberPwd_stateChanged(int state)
{
    //记住密码状态改变
    //qDebug() << __FILE__ << "(" << __LINE__ << "):";
    record->config()["remember"] = state == Qt::Checked;
    if(state == Qt::Checked)
    {
        //qDebug() << __FILE__ << "(" << __LINE__ << "):";
    }
    else
    {
        //ui->autoLoginCheck->setChecked(false);//关闭记住密码，则取消自动登录
        //qDebug() << __FILE__ << "(" << __LINE__ << "):";
    }
}

void LoginForm::slots_autoLoginCheck_stateChange(int state)
{
    //qDebug() << __FILE__ << "(" << __LINE__ << "):";
    record->config()["auto"] = state == Qt::Checked;
    if(state == Qt::Checked)
    {
        record->config()["remember"] = true;
        ui->remberPwd->setChecked(true);//自动登录会开启记住密码
        //ui->remberPwd->setCheckable(false);//禁止修改状态
        //qDebug() << __FILE__ << "(" << __LINE__ << "):";
    }
    else
    {
        ui->remberPwd->setCheckable(true);//启动修改状态
        //qDebug() << __FILE__ << "(" << __LINE__ << "):";
    }
    //qDebug() << __FILE__ << "(" << __LINE__ << "):";
}

void LoginForm::slots_login_request_finshed(QNetworkReply* reply)
{
    // 启用登录窗口以响应用户操作
    this->setEnabled(true);
    // 标志位，表示登录是否成功
    bool login_success = false;
    // 检查网络请求是否发生错误
    if (reply->error() != QNetworkReply::NoError)
    {
        // 如果发生错误，显示错误信息并返回
        info.set_text(u8"登录发生错误\r\n" + reply->errorString(), u8"确认").show();
        return;
    }
    // 读取服务器返回的数据
    QByteArray data = reply->readAll();
    qDebug() << data; // 输出数据以便调试
    // 解析 JSON 数据的错误信息
    QJsonParseError json_error;
    // 将服务器返回的 JSON 数据解析为 QJsonDocument 对象
    QJsonDocument document = QJsonDocument::fromJson(data, &json_error);
    qDebug() << "json error = " << json_error.error;
    // 检查是否解析成功
    if (json_error.error == QJsonParseError::NoError)
    {
        // 如果 JSON 数据是对象类型，进一步处理
        if (document.isObject())
        {
            // 获取 JSON 对象
            const QJsonObject obj = document.object();
            // 检查 JSON 对象中是否包含 "status" 和 "message" 字段
            if (obj.contains("status") && obj.contains("message"))
            {
                // 获取 "status" 和 "message" 字段的值
                QJsonValue status = obj.value("status");
                QJsonValue message = obj.value("message");
                // 检查登录状态
                if (status.toInt(-1) == 0) // 如果 status 值为 0，表示登录成功
                {
                    // TODO: 保存 token，用于后续保持在线状态
                    LOGIN_STATUS = status.toInt(-1) == 0; // 更新全局登录状态变量
                    // 触发登录成功信号，传递用户名和 token（这里 token 目前是空的）
                    emit login(record->config()["user"].toString(), QByteArray());
                    // 隐藏登录窗口
                    hide();
                    // 更新登录成功标志
                    login_success = true;
                    // 获取当前时间并格式化为字符串
                    char tm[64] = "";
                    time_t t;
                    time(&t);
                    strftime(tm, sizeof(tm), "%Y-%m-%d %H:%M:%S", localtime(&t));
                    // 更新配置中的登录时间
                    record->config()["date"] = QString(tm);
                    // 保存配置
                    record->save();
                }
            }
        }
    }
    else //if (document.isObject())
    {
        // 如果 JSON 数据解析失败，显示错误提示
        info.set_text(u8"登录失败\r\n无法解析服务器应答！", u8"确认").show();
    }
    // 如果登录未成功，显示用户名或密码错误提示
    if (!login_success) //login_success == false
    {
        info.set_text(u8"登录失败\r\n用户名或者密码错误！", u8"确认").show();
    }
    // 释放网络回复对象，避免内存泄漏
    reply->deleteLater();
}


QString getTime()
{
    time_t t = 0;
    time (&t);
    return QString::number(t);
}

bool LoginForm::check_login(const QString& user, const QString& pwd)
{
    //*
    QCryptographicHash md5(QCryptographicHash::Md5);
    QNetworkRequest request;
    QString url = QString(HOST) + "/login?";
    QString salt = QString::number(QRandomGenerator::global()->bounded(10000, 99999));
    QString time = getTime();
    qDebug().nospace()<< __FILE__ << "(" << __LINE__ << "):" <<time + MD5_KEY + pwd + salt;
    md5.addData((time + MD5_KEY + pwd + salt).toUtf8());
    QString sign = md5.result().toHex();
    //8C33079A8242BF2DEE2ED618FD7AA0C2
    qDebug()<<"sign:"<<sign;
    url += "time=" + time + "&";
    url += "salt=" + salt + "&";
    url += "user=" + user + "&";
    url += "sign=" + sign;
    qDebug() << url;
    request.setUrl(QUrl(url));
    record->config()["password"] = ui->pwdEdit->text();
    record->config()["user"] = ui->nameEdit->text();
    this->setEnabled(false);
    net->get(request);
    return true;
    //*/
    /*
    LOGIN_STATUS = true;
    emit login(record->config()["user"].toString(), QByteArray());
    hide();
    char tm[64] = "";
    time_t t;
    ::time(&t);
    strftime(tm, sizeof(tm), "%Y-%m-%d %H:%M:%S", localtime(&t));
    record->config()["date"] = QString(tm);//更新登录时间
    record->save();
    return false;
    //*/
}

void LoginForm::load_config()
{
    connect(ui->autoLoginCheck, SIGNAL(stateChanged(int)),
            this, SLOT(slots_autoLoginCheck_stateChange(int)));
    QJsonObject& root = record->config();
    ui->remberPwd->setChecked(root["remember"].toBool());
    ui->autoLoginCheck->setChecked(root["auto"].toBool());
    QString user = root["user"].toString();
    QString pwd = root["password"].toString();
    ui->nameEdit->setText(user);
    ui->pwdEdit->setText(pwd);
    qDebug() << "auto:" << root["auto"].toBool();
    qDebug() << "remember:" << root["remember"].toBool();
    if(root["auto"].toBool()) //如果开启了自动登录，则检查用户名和密码是否ok
    {
        qDebug() << __FILE__ << "(" << __LINE__ << "):user=" << user;
        qDebug() << __FILE__ << "(" << __LINE__ << "):pwd=" << pwd;
        if(user.size() > 0 && pwd.size() > 0)
        {
            ui->logoButton->setText(u8"取消自动登录");
            auto_login_id = startTimer(3000);//给3秒的时间，方便用户终止登录过程
        }
    }
}

void LoginForm::mouseMoveEvent(QMouseEvent* event)
{
    move(event->globalPos() - position);
}

void LoginForm::mousePressEvent(QMouseEvent* event)
{
    position = event->globalPos() - this->pos();
}

void LoginForm::mouseReleaseEvent(QMouseEvent* /*event*/)
{
}
void LoginForm::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape)  // 如果按下的是 Esc 键
    {
        if (escCount.load() == 0)
        {
            // 第一次按下 Esc 键，弹出提示框
            info.set_text(u8"再次点击 ESC 键退出本程序", u8"确认");
            info.show();
            escCount.store(1);  // 更新 escCount 为 1，表示第一次按下
        }
        else if (escCount.load() == 1)
        {
            // 第二次按下 Esc 键，退出程序
            QCoreApplication::quit();  // 或者 QApplication::exit();
        }
    }
    else
    {
        QWidget::keyPressEvent(event);  // 处理其他按键
    }
}

