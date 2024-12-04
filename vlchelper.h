#ifndef VLCHELPER_H
#define VLCHELPER_H

// 定义 ssize_t 类型（如果未定义）
#ifndef SSIZE_T
#define SSIZE_T
typedef long long ssize_t; // 使用 long long 定义 ssize_t 类型
#endif

#include "vlc.h"
#include <QString>
#include <QWidget>
#include <fstream>
#include "mediamp4.h" // MP4 媒体处理类

// vlcmedia 类用于处理 VLC 媒体的打开、读取和关闭等操作
class vlcmedia
{
public:
    vlcmedia(libvlc_instance_t* instance); // 构造函数，传入 VLC 实例
    ~vlcmedia(); // 析构函数，释放资源

    // 打开媒体文件
    static int open(void* thiz, void** infile, uint64_t* fsize);

    // 读取数据
    static ssize_t read(void* thiz, uint8_t* buffer, size_t length);

    // 移动文件指针
    static int seek(void* thiz, uint64_t offset);

    // 关闭媒体文件
    static void close(void* thiz);

    // 将 QString 类型的路径赋值给 vlcmedia 对象
    vlcmedia& operator=(const QString& str);

    // 转换操作符，返回 libvlc_media_t 指针
    operator libvlc_media_t* ();

    // 释放资源
    void free();

    // 获取媒体路径
    QString path();

private:
    // 私有方法
    int open(void** infile, uint64_t* fsize); // 打开文件
    ssize_t read(uint8_t* buffer, size_t length); // 读取数据
    int seek(uint64_t offset); // 移动指针
    void close(); // 关闭文件

private:
    QString strPath; // 媒体文件路径
    libvlc_media_t* media; // VLC 媒体指针
    std::ifstream infile; // 输入文件流
    libvlc_instance_t* instance; // VLC 实例
    uint64_t media_size; // 媒体文件大小
    MediaBase* media_instance; // 媒体实例
};

// vlchelper 类用于控制 VLC 媒体播放器的操作
class vlchelper
{
public:
    vlchelper(QWidget* widget); // 构造函数，传入 Qt 窗口部件
    ~vlchelper(); // 析构函数，释放资源

    // 准备播放媒体文件
    int prepare(const QString& strPath = "");

    // 播放媒体
    int play();

    // 暂停播放
    int pause();

    // 停止播放
    int stop();

    // 设置音量
    int volume(int vol = -1);

    // 静音
    int silence();

    // 检查是否正在播放
    bool isplaying();

    // 检查是否处于暂停状态
    bool ispause() const { return m_ispause; }

    // 检查是否静音
    bool ismute();

    // 获取当前播放时间（毫秒）
    libvlc_time_t gettime();

    // 获取媒体总时长（毫秒）
    libvlc_time_t getduration();

    // 设置播放时间
    int settime(libvlc_time_t time);

    // 设置播放速率
    int set_play_rate(float rate);

    // 获取播放速率
    float get_play_rate();

    // 初始化 logo
    void init_logo();

    // 更新 logo
    void update_logo();

    // 检查 logo 是否启用
    bool is_logo_enable();

    // 初始化文本
    void init_text(const QString& text);

    // 更新文本
    void update_text();

    // 检查文本是否启用
    bool is_text_enable();

    // 检查是否有媒体正在播放
    bool has_media_player();

private:
    void set_float_text(); // 设置浮动文本

private:
    libvlc_instance_t* m_instance; // VLC 实例指针
    libvlc_media_player_t* m_player; // VLC 媒体播放器指针
    HWND m_hWnd; // 窗口句柄
    int winHeight; // 窗口高度
    int winWidth; // 窗口宽度
    bool m_isplaying; // 是否正在播放
    bool m_ispause; // 是否暂停
    bool m_issilence; // 是否静音
    int m_volume; // 当前音量
    libvlc_time_t m_duration; // 当前媒体总时长（毫秒）
    vlcmedia* m_media; // VLC 媒体实例
    QFile m_logo; // logo 文件
    QString text; // 文本
    QWidget* m_widget; // Qt 窗口部件指针
};

#endif // VLCHELPER_H
