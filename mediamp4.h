#ifndef MEDIAMP4_H
#define MEDIAMP4_H

#include "mediabase.h"

// MediaMP4 类用于处理 MP4 媒体文件，继承自 MediaBase 类
class MediaMP4: public MediaBase
{
public:
    MediaMP4(); // 构造函数
    virtual ~MediaMP4(); // 虚析构函数，释放资源

    // 打开指定的媒体文件，返回文件大小
    virtual int open(const QUrl& url, uint64_t* fsize);

    // 从媒体文件中读取数据，返回实际读取的字节数
    virtual ssize_t read(uint8_t* buffer, size_t length);

    // 移动文件指针到指定偏移量
    virtual int seek(uint64_t offset);

    // 关闭媒体文件
    virtual void close();

private:
    // 解密数据的方法
    void decrypto(QByteArray& data, uint64_t position);

private:
    QByteArray key; // 存储用于解密的密钥
    QFile* file; // 文件指针，用于操作 MP4 文件
    uint16_t pos; // 当前读取位置
};

#endif // MEDIAMP4_H
