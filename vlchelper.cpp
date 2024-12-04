#include "vlchelper.h"
#include <functional>
#include <QDebug>
#include <QTime>
#include <QRandomGenerator>
#include <QImage>
//vlc 播放进度回调
//拖拽进度、播放速度、停止播放
using namespace std;
using namespace std::placeholders;
void libvlc_log_callback(void* data, int level, const libvlc_log_t* ctx,
                         const char* fmt, va_list args)
{
    qDebug() << "log level:" << level;
}


// vlchelper 构造函数
vlchelper::vlchelper(QWidget* widget)
    : m_logo(":/ico/UI/ico/128-128.png") // 初始化 m_logo，加载资源中的图标路径
{
    // VLC 初始化参数：指定启用 logo 和 marq（字幕/水印）过滤器
    const char* const args[] =
        {
            "--sub-filter=logo", // 启用 logo 滤镜
            "--sub-filter=marq"  // 启用字幕标记滤镜
        };

    qDebug() << __FUNCTION__ << ":" << __LINE__; // 输出调试信息，表示进入构造函数

    // 创建 VLC 实例
    m_instance = libvlc_new(2, args); // 2 表示传递的参数个数，args 是参数数组
    if (m_instance != NULL) // 检查 VLC 实例是否创建成功
    {
        qDebug() << __FUNCTION__ << ":" << __LINE__; // 输出成功创建实例的调试信息
        m_media = new vlcmedia(m_instance); // 创建媒体对象
    }
    else
    {
        qDebug() << __FUNCTION__ << ":" << __LINE__; // 输出实例创建失败的调试信息
        m_media = NULL; // 如果实例创建失败，设置 m_media 为 NULL
        throw QString("是否没有安装插件？！！"); // 抛出异常提示插件可能缺失
    }

    // 设置 VLC 的日志回调函数，用于调试或记录日志
    libvlc_log_set(m_instance, libvlc_log_callback, this);

    m_player = NULL; // 初始化播放器对象为 NULL
    m_hWnd = (HWND)widget->winId(); // 获取 Qt 窗口的原生窗口句柄
    winHeight = widget->frameGeometry().height(); // 获取窗口高度
    winWidth = widget->frameGeometry().width(); // 获取窗口宽度
    m_widget = widget; // 保存 QWidget 指针

    // 输出窗口的宽高调试信息
    qDebug() << "*winWidth:" << winWidth;
    qDebug() << "*winHeight:" << winHeight;

    // 初始化播放状态
    m_isplaying = false; // 默认未播放
    m_ispause = false; // 默认未暂停
    m_volume = 100; // 初始化音量为 100（最大音量）
}

// vlchelper 析构函数
vlchelper::~vlchelper()
{
    // 如果播放器已创建，释放相关资源
    if (m_player != NULL)
    {
        stop(); // 停止播放
        libvlc_media_player_set_hwnd(m_player, NULL); // 解绑窗口句柄
        libvlc_media_player_release(m_player); // 释放媒体播放器对象
        m_player = NULL; // 置空指针
    }

    // 释放媒体对象资源
    if (m_media != NULL)
    {
        m_media->free(); // 调用 vlcmedia 的资源释放函数
        m_media = NULL; // 置空指针
    }

    // 释放 VLC 实例资源
    if (m_instance != NULL)
    {
        libvlc_release(m_instance); // 释放 VLC 实例
        m_instance = NULL; // 置空指针
    }
}


/**
 * @brief 准备媒体播放。
 *
 * 此函数用于初始化媒体播放器，加载指定的媒体文件，并进行必要的设置。
 *
 * @param strPath 媒体文件的路径，支持本地文件和网络地址。
 * @return int 返回值：
 *         - 0：成功准备媒体。
 *         - -1：媒体对象创建失败。
 *         - -2：媒体播放器创建失败。
 *
 * 此函数的主要步骤包括：
 * 1. 打印调试信息，输出传入的媒体路径。
 * 2. 使用提供的媒体路径创建媒体对象。
 * 3. 如果媒体对象创建失败，返回 -1。
 * 4. 如果已有播放器实例，释放该实例。
 * 5. 创建新的媒体播放器实例。
 * 6. 如果播放器创建失败，返回 -2。
 * 7. 获取媒体的持续时间，并保存到成员变量。
 * 8. 设置播放器的窗口句柄，以便在指定的窗口中播放媒体。
 * 9. 设置音量和视频宽高比。
 * 10. 如果有文本内容，调用设置悬浮文字的函数。
 * 11. 初始化播放、暂停和静音状态。
 * 12. 更新窗口高度和宽度。
 * 13. 输出当前窗口的宽高调试信息。
 */
int vlchelper::prepare(const QString& strPath)
{
    qDebug() << strPath; // 输出传入的媒体路径以便调试

    // 创建媒体对象，使用指定的媒体路径
    // m_media = libvlc_media_new_location(m_instance, strPath.toStdString().c_str());
    *m_media = strPath; // 将媒体路径赋值给媒体对象
    if(m_media == NULL) // 检查媒体对象是否成功创建
    {
        return -1; // 返回 -1 表示媒体对象创建失败
    }

    // 如果已有播放器实例，释放该实例
    if(m_player != NULL)
    {
        libvlc_media_player_release(m_player); // 释放媒体播放器
    }

    // 创建新的媒体播放器实例
    m_player = libvlc_media_player_new_from_media(*m_media);
    if(m_player == NULL) // 检查媒体播放器是否成功创建
    {
        return -2; // 返回 -2 表示媒体播放器创建失败
    }

    // 获取媒体的持续时间
    m_duration = libvlc_media_get_duration(*m_media);
    // 设置播放器的窗口句柄
    libvlc_media_player_set_hwnd(m_player, m_hWnd);
    // 设置音量
    libvlc_audio_set_volume(m_player, m_volume);
    // 设置视频宽高比
    libvlc_video_set_aspect_ratio(m_player, "16:9");

    // 如果有文本内容，设置悬浮文字
    if(text.size() > 0)
    {
        set_float_text();
    }

    // 初始化播放、暂停和静音状态
    m_ispause = false; // 默认未暂停
    m_isplaying = false; // 默认未播放
    m_issilence = false; // 默认无静音状态

    // 更新窗口高度和宽度
    if(m_widget->frameGeometry().height() != winHeight)
    {
        winHeight = m_widget->frameGeometry().height(); // 更新窗口高度
    }
    if(m_widget->frameGeometry().width() != winWidth)
    {
        winWidth = m_widget->frameGeometry().width(); // 更新窗口宽度
    }

    // 输出当前窗口的宽高以便调试
    qDebug() << "*winWidth:" << winWidth;
    qDebug() << "*winHeight:" << winHeight;

    return 0; // 返回 0 表示成功准备媒体
}

/**
 * @brief 播放媒体。
 *
 * 此函数用于播放当前媒体。如果媒体处于暂停状态，则恢复播放。
 *
 * @return int 返回值：
 *         - 0：播放成功。
 *         - -1：播放器未初始化或媒体未设置。
 *         - -2：未设置媒体路径。
 *
 * 此函数的主要步骤包括：
 * 1. 检查播放器是否已初始化，如果未初始化，返回 -1。
 * 2. 检查媒体是否处于暂停状态，如果是，则调用播放函数恢复播放，并更新状态。
 * 3. 如果播放器未初始化或媒体路径为空，更新状态并返回 -2。
 * 4. 更新播放状态为正在播放，并禁用 VLC 对鼠标和键盘的处理，以便 Qt 处理交互。
 * 5. 设置播放器为全屏模式。
 * 6. 调用播放函数开始播放媒体。
 */
int vlchelper::play()
{
    if(m_player == NULL) // 检查播放器是否已初始化
    {
        return -1; // 返回 -1 表示播放器未初始化
    }

    if(m_ispause) // 如果媒体处于暂停状态
    {
        int ret = libvlc_media_player_play(m_player); // 恢复播放
        if(ret == 0) // 如果恢复播放成功
        {
            m_ispause = false; // 更新状态为未暂停
            m_isplaying = true; // 更新状态为正在播放
        }
        return ret; // 返回执行结果
    }

    // 检查媒体是否未设置或路径为空
    if((m_player == NULL) || (m_media->path().size() <= 0))
    {
        m_ispause = false; // 更新状态为未暂停
        m_isplaying = false; // 更新状态为未播放
        return -2; // 返回 -2 表示未设置媒体路径
    }

    m_isplaying = true; // 更新播放状态为正在播放
    libvlc_video_set_mouse_input(m_player, 0); // 禁用 VLC 的鼠标输入处理
    libvlc_video_set_key_input(m_player, 0); // 禁用 VLC 的键盘输入处理
    libvlc_set_fullscreen(m_player, 1); // 设置全屏模式
    return libvlc_media_player_play(m_player); // 开始播放媒体
}

/**
 * @brief 暂停媒体播放。
 *
 * 此函数用于暂停当前正在播放的媒体。
 *
 * @return int 返回值：
 *         - 0：暂停成功。
 *         - -1：播放器未初始化。
 *
 * 此函数的主要步骤包括：
 * 1. 检查播放器是否已初始化，如果未初始化，返回 -1。
 * 2. 调用暂停函数暂停媒体播放，并更新暂停状态。
 */
int vlchelper::pause()
{
    if(m_player == NULL) // 检查播放器是否已初始化
    {
        return -1; // 返回 -1 表示播放器未初始化
    }

    libvlc_media_player_pause(m_player); // 暂停媒体播放
    m_ispause = true; // 更新状态为已暂停
    m_isplaying = false; // 更新状态为未播放
    return 0; // 返回 0 表示暂停成功
}

/**
 * @brief 停止媒体播放。
 *
 * 此函数用于停止当前正在播放的媒体。
 *
 * @return int 返回值：
 *         - 0：停止成功。
 *         - -1：播放器未初始化。
 *
 * 此函数的主要步骤包括：
 * 1. 检查播放器是否已初始化，如果已初始化，调用停止函数停止播放，并更新状态。
 */
int vlchelper::stop()
{
    if(m_player != NULL) // 检查播放器是否已初始化
    {
        libvlc_media_player_stop(m_player); // 停止媒体播放
        m_isplaying = false; // 更新状态为未播放
        return 0; // 返回 0 表示停止成功
    }
    return -1; // 返回 -1 表示播放器未初始化
}

/**
 * @brief 设置或获取音量。
 *
 * 此函数用于设置媒体播放器的音量。如果传入的音量为 -1，则返回当前音量。
 *
 * @param vol 要设置的音量，范围为 0 到 100。传入 -1 表示获取当前音量。
 * @return int 返回值：
 *         - 当前音量（如果 vol 为 -1）。
 *         - -1：播放器未初始化。
 *         - 其他值：设置音量的结果（0 表示成功）。
 *
 * 此函数的主要步骤包括：
 * 1. 检查播放器是否已初始化，如果未初始化，返回 -1。
 * 2. 如果 vol 为 -1，返回当前音量。
 * 3. 设置播放器音量，并更新成员变量，如果设置成功，返回当前音量。
 */
int vlchelper::volume(int vol)
{
    if(m_player == NULL) // 检查播放器是否已初始化
    {
        return -1; // 返回 -1 表示播放器未初始化
    }

    if(vol == -1) // 如果 vol 为 -1，返回当前音量
    {
        return m_volume; // 返回当前音量
    }

    int ret = libvlc_audio_set_volume(m_player, vol); // 设置音量
    if(ret == 0) // 如果设置成功
    {
        m_volume = vol; // 更新成员变量
        return m_volume; // 返回当前音量
    }

    return ret; // 返回设置音量的结果
}


int vlchelper::silence()
{
    if(m_player == NULL)
    {
        return -1;
    }
    if(m_issilence)
    {
        //恢复
        libvlc_audio_set_mute(m_player, 0);
        m_issilence = false;
    }
    else
    {
        //静音
        m_issilence = true;
        libvlc_audio_set_mute(m_player, 1);
    }
    return m_issilence;
}

bool vlchelper::isplaying()
{
    return m_isplaying;
}

bool vlchelper::ismute()
{
    if(m_player && m_isplaying)
    {
        return libvlc_audio_get_mute(m_player) == 1;
    }
    return false;
}

libvlc_time_t vlchelper::gettime()
{
    if(m_player == NULL)
    {
        return -1;
    }
    return libvlc_media_player_get_time(m_player);
}

libvlc_time_t vlchelper::getduration()
{
    if(m_media == NULL)
    {
        return -1;
    }
    if(m_duration == -1)
    {
        m_duration = libvlc_media_get_duration(*m_media);
    }
    return m_duration;
}

int vlchelper::settime(libvlc_time_t time)
{
    if(m_player == NULL)
    {
        return -1;
    }
    libvlc_media_player_set_time(m_player, time);
    return 0;
}

int vlchelper::set_play_rate(float rate)
{
    if(m_player == NULL)
    {
        return -1;
    }
    return libvlc_media_player_set_rate(m_player, rate);
}

float vlchelper::get_play_rate()
{
    if(m_player == NULL)
    {
        return -1.0;
    }
    return libvlc_media_player_get_rate(m_player);
}

void vlchelper::init_logo()
{
    //libvlc_video_set_logo_int(m_player, libvlc_logo_file, m_logo.handle());
    libvlc_video_set_logo_string(m_player, libvlc_logo_file, "128-128.png"); //Logo 文件名
    libvlc_video_set_logo_int(m_player, libvlc_logo_x, 0); //logo的 X 坐标。
    //libvlc_video_set_logo_int(m_player, libvlc_logo_y, 0); // logo的 Y 坐标。
    libvlc_video_set_logo_int(m_player, libvlc_logo_delay, 100);//标志的间隔图像时间为毫秒,图像显示间隔时间 0 - 60000 毫秒。
    libvlc_video_set_logo_int(m_player, libvlc_logo_repeat, -1); // 标志logo的循环,  标志动画的循环数量。-1 = 继续, 0 = 关闭
    libvlc_video_set_logo_int(m_player, libvlc_logo_opacity, 122);
    // logo 透明度 (数值介于 0(完全透明) 与 255(完全不透明)
    libvlc_video_set_logo_int(m_player, libvlc_logo_position, 5);
    //1 (左), 2 (右), 4 (顶部), 8 (底部), 5 (左上), 6 (右上), 9 (左下), 10 (右下),您也可以混合使用这些值，例如 6=4+2    表示右上)。
    libvlc_video_set_logo_int(m_player, libvlc_logo_enable, 1); //设置允许添加logo
}

void vlchelper::init_text(const QString& text)
{
    this->text = text;
}

void vlchelper::update_logo()
{
    static int alpha = 0;
    //static int pos[] = {1, 5, 4, 6, 2, 10, 8, 9};
    int height = QRandomGenerator::global()->bounded(20, winHeight - 20);
    libvlc_video_set_logo_int(m_player, libvlc_logo_y, height); // logo的 Y 坐标。
    int width = QRandomGenerator::global()->bounded(20, winWidth - 20);
    libvlc_video_set_logo_int(m_player, libvlc_logo_x, width); //logo的 X 坐标。
    libvlc_video_set_logo_int(m_player, libvlc_logo_opacity, (alpha++) % 80 + 20); //透明度
    //libvlc_video_set_logo_int(m_player, libvlc_logo_position, pos[alpha % 8]);
}

void vlchelper::update_text()
{
    static int alpha = 0;
    if(m_player)
    {
        int color = QRandomGenerator::global()->bounded(0x30, 0x60);//R
        color = color << 8 | QRandomGenerator::global()->bounded(0x30, 0x60);//G
        color = color << 8 | QRandomGenerator::global()->bounded(0x30, 0x60);//B
        libvlc_video_set_marquee_int(m_player, libvlc_marquee_Color, color);//颜色
        int x = QRandomGenerator::global()->bounded(20, winHeight - 20);//随机位置
        int y = QRandomGenerator::global()->bounded(20, winWidth - 20);
        libvlc_video_set_marquee_int(m_player, libvlc_marquee_X, x);//
        libvlc_video_set_marquee_int(m_player, libvlc_marquee_Y, y);
        //随机透明度
        libvlc_video_set_marquee_int(m_player, libvlc_marquee_Opacity, (alpha++ % 60) + 10);
    }
}

bool vlchelper::is_text_enable()
{
    if(m_player == NULL)
    {
        return false;
    }
    return libvlc_video_get_marquee_int(m_player, libvlc_marquee_Enable) == 1;
}

bool vlchelper::has_media_player()
{
    return m_player != NULL && (m_media != NULL);
}

void vlchelper::set_float_text()
{
    if(m_player)
    {
        libvlc_video_set_marquee_string(m_player, libvlc_marquee_Text, text.toStdString().c_str());
        libvlc_video_set_marquee_int(m_player, libvlc_marquee_Color, 0x404040);
        libvlc_video_set_marquee_int(m_player, libvlc_marquee_X, 0);
        libvlc_video_set_marquee_int(m_player, libvlc_marquee_Y, 0);
        libvlc_video_set_marquee_int(m_player, libvlc_marquee_Opacity, 100);
        //libvlc_video_set_marquee_int(m_player, libvlc_marquee_Timeout, 100);
        libvlc_video_set_marquee_int(m_player, libvlc_marquee_Position, 5);
        libvlc_video_set_marquee_int(m_player, libvlc_marquee_Size, 14);
        libvlc_video_set_marquee_int(m_player, libvlc_marquee_Enable, 1);
    }
}

bool vlchelper::is_logo_enable()
{
    if(m_player == NULL)
    {
        return false;
    }
    qDebug() << __FUNCTION__ << " logo enable:" << libvlc_video_get_logo_int(m_player, libvlc_logo_enable);
    return libvlc_video_get_logo_int(m_player, libvlc_logo_enable) == 1;
}

vlcmedia::vlcmedia(libvlc_instance_t* ins)
    : instance(ins)
{
    media = NULL;
    media_instance = new MediaMP4();
}

vlcmedia::~vlcmedia()
{
    if(media)
    {
        free();
    }
    if(media_instance)
    {
        delete media_instance;
        media_instance = NULL;
    }
}

int vlcmedia::open(void* thiz, void** infile, uint64_t* fsize)
{
    vlcmedia* _this = (vlcmedia*)thiz;
    return _this->open(infile, fsize);
}

ssize_t vlcmedia::read(void* thiz, uint8_t* buffer, size_t length)
{
    vlcmedia* _this = (vlcmedia*)thiz;
    return _this->read(buffer, length);
}

int vlcmedia::seek(void* thiz, uint64_t offset)
{
    vlcmedia* _this = (vlcmedia*)thiz;
    return _this->seek(offset);
}

void vlcmedia::close(void* thiz)
{
    vlcmedia* _this = (vlcmedia*)thiz;
    _this->close();
}

vlcmedia& vlcmedia::operator=(const QString& str)
{
    if(media)
    {
        free();
    }
    //libvlc_media_read_cb
    strPath = str;
    media = libvlc_media_new_callbacks(
                instance,
                &vlcmedia::open,
                &vlcmedia::read,
                &vlcmedia::seek,
                &vlcmedia::close,
                this);
    return *this;
}

void vlcmedia::free()
{
    if(media != NULL)
    {
        libvlc_media_release(media);
    }
}

QString vlcmedia::path()
{
    return strPath;
}

int vlcmedia::open(void** infile, uint64_t* fsize)
{
    //"file:///"
    if(media_instance)
    {
        *infile = this;
        int ret = media_instance->open(strPath, fsize);
        media_size = *fsize;
        return ret;
    }
    this->infile.open(strPath.toStdString().c_str() + 8, ios::binary | ios::in);
    this->infile.seekg(0, ios::end);
    *fsize = (uint64_t)this->infile.tellg();
    media_size = *fsize;
    this->infile.seekg(0);
    *infile = this;
    return 0;
}

ssize_t vlcmedia::read(uint8_t* buffer, size_t length)
{
    if(media_instance)
    {
        return media_instance->read(buffer, length);
    }
    //qDebug() << __FUNCTION__ << " length:" << length;
    uint64_t pos = (uint64_t)infile.tellg();
    //qDebug() << __FUNCTION__ << " positon:" << pos;
    if(media_size - pos < length)
    {
        length = media_size - pos;
    }
    infile.read((char*)buffer, length);
    return infile.gcount();
}

int vlcmedia::seek(uint64_t offset)
{
    if(media_instance)
    {
        return media_instance->seek(offset);
    }
    //qDebug() << __FUNCTION__ << ":" << offset;
    infile.clear();
    infile.seekg(offset);
    return 0;
}

void vlcmedia::close()
{
    if(media_instance)
    {
        return media_instance->close();
    }
    //qDebug() << __FUNCTION__;
    infile.close();
}

vlcmedia::operator libvlc_media_t* ()
{
    return media;
}
