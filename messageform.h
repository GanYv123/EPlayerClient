#ifndef MESSAGEFORM_H
#define MESSAGEFORM_H

#include <QWidget>   // 基类 QWidget 的头文件
#include <QImage>    // 用于图像处理的类

namespace Ui
{
class MessageForm; // 前向声明 UI 类
}

/**
 * @class MessageForm
 * @brief 一个无边框、透明背景的窗口，用于显示特定的图像和文本。
 */
class MessageForm : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针，默认为 nullptr
     */
    explicit MessageForm(QWidget* parent = nullptr);

    /**
     * @brief 析构函数，释放动态分配的资源
     */
    ~MessageForm();

    /**
     * @brief 设置背景窗口指针，用于截图
     * @param bkgnd 背景窗口的指针
     */
    void set_widget(QWidget* bkgnd)
    {
        backgnd = bkgnd;
    }

    /**
     * @brief 设置窗口全屏宽度
     * 将窗口宽度设置为屏幕宽度，高度固定为 200。
     */
    void full_size()
    {
        setGeometry(0, 0, screen_width, 200);
    }

    /**
     * @brief 设置显示的文本
     * @param text 要显示的文本
     */
    void setText(const QString& text);

    /**
     * @brief 将 HBITMAP 转换为 QImage
     * @param hBitmap Windows API 的 HBITMAP 位图句柄
     * @return 转换后的 QImage 对象
     */
    QImage hBitmapToQImage(HBITMAP hBitmap);

protected:
    /**
     * @brief 重写 paintEvent 事件，绘制窗口内容
     * @param event 事件参数
     */
    virtual void paintEvent(QPaintEvent* event);

    /**
     * @brief 截取指定窗口的内容并转换为 QImage
     * @param hWnd 窗口句柄
     * @return 截取后的 QImage
     */
    QImage CopyDCToBitmap(HWND hWnd);

private:
    Ui::MessageForm* ui;  ///< UI 界面指针
    QWidget* backgnd;     ///< 背景窗口指针
    QImage* top;          ///< 用于显示的顶部图像
    int screen_width;     ///< 屏幕宽度
    QString text;         ///< 显示的文本
};

#endif // MESSAGEFORM_H
