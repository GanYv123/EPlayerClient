#include "messageform.h"
#include "ui_messageform.h"
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QScreen>
#include <Windows.h> // 包含 Windows API，用于 HBITMAP 和 HWND 的操作

RECT rect;

MessageForm::MessageForm(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::MessageForm)
{
    ui->setupUi(this);

    // 设置窗口无边框
    setWindowFlags(Qt::FramelessWindowHint);

    // 设置窗口背景透明
    setAttribute(Qt::WA_TranslucentBackground);

    // 加载顶部图片资源
    top = new QImage(":/UI/images/screentop.png");

    // 获取屏幕宽度，替代已废弃的 QApplication::desktop()
    screen_width = QGuiApplication::primaryScreen()->size().width();
}

MessageForm::~MessageForm()
{
    delete ui;
    delete top; // 清理动态分配的 QImage
}

// 设置显示文本
void MessageForm::setText(const QString& text)
{
    this->text = text;
}

// 绘制事件处理
void MessageForm::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    // 获取窗口几何信息
    QRect r = frameGeometry();

    // 定义用于捕获窗口截图的点
    POINT pt = {r.right() / 2, r.bottom() + 5};
    HWND hWndBack = WindowFromPoint(pt);

    // 截取指定窗口的图像
    QImage image = CopyDCToBitmap(hWndBack);

    // 设置字体样式
    QFont font = painter.font();
    font.setPixelSize(18);
    painter.setFont(font);

    // 设置组合模式
    painter.setCompositionMode(QPainter::CompositionMode_Source);

    // 绘制捕获的窗口图像
    painter.drawImage(0, 0, image, 0, 0);

    // 绘制顶部图片
    painter.drawImage(0, 0, *top, 0, 0);

    // 设置文本颜色
    QPen pen = painter.pen();
    pen.setColor(QColor(255, 255, 255));
    painter.setPen(pen);

    // 调整文本绘制区域
    r.setLeft(r.left() + 50);
    r.setTop(r.top() + 50);

    // 绘制文本
    painter.drawText(r, text);
}

// 将 HBITMAP 转换为 QImage
QImage MessageForm::CopyDCToBitmap(HWND hWnd)
{
    // 获取窗口的设备上下文
    HDC hDC = GetWindowDC(hWnd);
    if (!hDC)
    {
        return QImage(); // 如果获取失败，返回空图像
    }

    // 获取窗口矩形尺寸
    RECT wndRect;
    GetWindowRect(hWnd, &wndRect);
    int nWidth = wndRect.right - wndRect.left;
    int nHeight = wndRect.bottom - wndRect.top;

    // 创建兼容的内存设备上下文和位图
    HDC hMemDC = CreateCompatibleDC(hDC);
    HBITMAP hBitmap = ::CreateCompatibleBitmap(hDC, nWidth, nHeight);
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemDC, hBitmap);

    // 捕获窗口内容到内存设备上下文
    ::PrintWindow(hWnd, hMemDC, PW_CLIENTONLY | PW_RENDERFULLCONTENT);

    // 恢复旧的位图并释放内存设备上下文
    SelectObject(hMemDC, hOldBitmap);
    DeleteDC(hMemDC);

    // 将 HBITMAP 转换为 QImage
    QImage img = hBitmapToQImage(hBitmap);

    // 删除 HBITMAP 以释放资源
    DeleteObject(hBitmap);

    return img;
}

// 实现 HBITMAP 到 QImage 的转换
QImage MessageForm::hBitmapToQImage(HBITMAP hBitmap)
{
    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    int width = bmp.bmWidth;
    int height = bmp.bmHeight;

    QImage img(width, height, QImage::Format_ARGB32);

    HDC hdc = CreateCompatibleDC(nullptr);
    HBITMAP oldBmp = (HBITMAP)SelectObject(hdc, hBitmap);

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // Top-down DIB
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    // 将位图数据复制到 QImage
    GetDIBits(hdc, hBitmap, 0, height, img.bits(), &bmi, DIB_RGB_COLORS);

    SelectObject(hdc, oldBmp);
    DeleteDC(hdc);

    return img;
}
