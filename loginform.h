#ifndef LOGINFORM_H
#define LOGINFORM_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QDesktopServices>
#include <QMouseEvent>

#include "infoform.h"
#include "recordfile.h"

// 命名空间 Ui 包含由 Qt Designer 生成的类声明
namespace Ui
{
class LoginForm; // 前向声明由 UI 文件生成的类
}

/**
 * @class LoginForm
 * @brief 用户登录窗口类，负责实现登录界面逻辑和用户交互。
 *
 * LoginForm 继承自 QWidget，用于处理登录界面的各种操作，如
 * 用户名、密码输入框处理，记住密码逻辑，自动登录处理等。
 */
class LoginForm : public QWidget
{
    Q_OBJECT
    friend class testVideoPlayer; // 声明 testVideoPlayer 类为友元类，允许其访问私有成员
public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit LoginForm(QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~LoginForm();

    /**
     * @brief 事件过滤器
     * @param watched 被监视的对象
     * @param event 捕获的事件
     * @return true 表示事件被处理，false 表示继续传播
     */
    virtual bool eventFilter(QObject* watched, QEvent* event);

    /**
     * @brief 定时器事件处理函数
     * @param event 定时器事件
     */
    virtual void timerEvent(QTimerEvent* event);

protected slots:
    /**
     * @brief 处理 logo 按钮释放事件
     */
    void on_logoButton_released();

    /**
     * @brief 处理“记住密码”复选框状态改变
     * @param state 复选框状态
     */
    void on_remberPwd_stateChanged(int state);

    /**
     * @brief 自动登录复选框状态改变槽函数
     * @param state 复选框状态
     */
    void slots_autoLoginCheck_stateChange(int state);

    /**
     * @brief 登录请求完成槽函数
     * @param reply 网络请求回复
     */
    void slots_login_request_finshed(QNetworkReply* reply);

signals:
    /**
     * @brief 登录成功信号
     * @param nick 用户昵称
     * @param head 用户头像数据
     */
    void login(const QString& nick, const QByteArray& head);

protected:
    /**
     * @brief 鼠标移动事件处理
     * @param event 鼠标事件
     */
    void mouseMoveEvent(QMouseEvent* event) override;

    /**
     * @brief 鼠标按下事件处理
     * @param event 鼠标事件
     */
    void mousePressEvent(QMouseEvent* event) override;

    /**
     * @brief 鼠标释放事件处理
     * @param event 鼠标事件
     */
    void mouseReleaseEvent(QMouseEvent* event) override;

    /**
     * @brief 键盘按下事件处理
     * @param event 键盘按下事件
     */
    void keyPressEvent(QKeyEvent* event) override;

private:
    /**
     * @brief 验证用户名和密码的有效性
     * @param user 用户名
     * @param pwd 密码
     * @return true 表示验证通过，false 表示验证失败
     */
    bool check_login(const QString& user, const QString& pwd);

    /**
     * @brief 加载登录界面的配置，例如记住密码、自动登录选项
     */
    void load_config();

private:
    Ui::LoginForm* ui;               ///< 指向 UI 对象的指针
    QNetworkAccessManager* net;      ///< 网络访问管理器，用于处理网络请求
    InfoForm info;                   ///< 子窗口，用于显示额外的信息
    RecordFile* record;              ///< 记录文件，用于保存用户登录数据
    int auto_login_id;               ///< 自动登录的定时器 ID
    QPoint position;                 ///< 鼠标按下时的窗口位置
    std::atomic<int> escCount;       ///< 按下两次esc时退出程序
};

#endif // LOGINFORM_H
