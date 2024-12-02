#include "recordfile.h"
#include <QFile>
#include <QDebug>

RecordFile::RecordFile(const QString& path)
{
    QFile file(path); // 创建一个 QFile 对象，用于操作指定路径的文件
    m_path = path;    // 保存传入的文件路径
    qDebug() << m_path; // 输出文件路径到调试控制台
    do
    {
        // 尝试打开文件以读取内容
        if(!file.open(QIODevice::ReadOnly))
        {
            break;  // 打开文件失败，跳出循环
        }

        QByteArray data = file.readAll(); // 读取文件中的所有数据
        if(data.size() <= 0)
        {
            break;  // 如果文件为空，跳出循环
        }

        // 使用工具类进行 RSA 解密
        data = tool.rsaDecode(data);

        // 遍历数据，找到数据中的有效部分（假设有效数据在 0x0A 到 0x7F 范围内）
        int i = 0;
        for(; i < data.size(); i++)
        {
            if((int)data[i] >= (int)0x7F || (int)data[i] < (int)0x0A)
            {
                data.resize(i);  // 截断数据，保留有效部分
                break;
            }
        }

        // 使用 QJsonDocument 将解密后的数据解析为 JSON 文档
        QJsonParseError json_error;
        QJsonDocument document = QJsonDocument::fromJson(data, &json_error);

        // 检查 JSON 数据是否解析成功
        if (json_error.error == QJsonParseError::NoError)
        {
            if (document.isObject()) // 如果解析成功并且是 JSON 对象
            {
                m_config = document.object(); // 将 JSON 对象赋值给成员变量 m_config
                return; // 配置读取成功，直接返回
            }
        }
        else
        {
            qDebug().nospace() << __FILE__ << "(" << __LINE__ << "):" << json_error.errorString() << json_error.error;
            // 如果解析失败，输出错误信息
        }
    }
    while(false);

    // 如果读取失败或解析失败，设置默认配置
    file.close();  // 关闭文件
    QJsonValue value = QJsonValue();

    // 设置默认的配置值
    m_config.insert("user", value);
    m_config.insert("password", value);
    m_config.insert("auto", false); // 自动登录（默认关闭）
    m_config.insert("remember", false); // 记住密码（默认关闭）
    m_config.insert("date", "2021-04-01 10:10:10"); // 默认日期

    return; // 默认配置已设置，返回
}

RecordFile::~RecordFile()
{
    // 析构函数，在对象销毁时保存配置
    //qDebug() << __FILE__ << "(" << __LINE__ << "):";
    save(); // 调用 save() 方法将配置保存到文件
}

QJsonObject& RecordFile::config()
{
    // 返回 m_config 配置对象的引用
    return m_config;
}

bool RecordFile::save()
{
    QFile file(m_path); // 创建一个 QFile 对象，用于操作配置文件
    if(file.open(QIODevice::WriteOnly | QIODevice::Truncate) == false)
    {
        // 如果文件打开失败，返回 false
        //qDebug() << __FILE__ << "(" << __LINE__ << "):";
        return false;
    }

    // 使用 QJsonDocument 将 m_config 转换为 JSON 数据
    QJsonDocument document = QJsonDocument(m_config);

    // 将 JSON 数据进行 RSA 加密
    file.write(tool.rsaEncode(document.toJson()));

    file.close();  // 关闭文件
    //qDebug() << __FILE__ << "(" << __LINE__ << "):" << m_config;
    return true; // 返回 true，表示保存成功
}
