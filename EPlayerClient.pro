QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# 添加 OpenSSL 的路径
INCLUDEPATH += C:/src/vcpkg/installed/x64-windows-static/include
LIBS += -LC:/src/vcpkg/installed/x64-windows-static/lib
LIBS += -llibcrypto -llibssl -lws2_32 -lbcrypt -lgdi32
LIBS += -lcrypt32 -luser32 -lkernel32 -ladvapi32


# 判断是否为64位构建套件 vlc
contains(QT_ARCH, x86_64) {
    # 设置 VLC 头文件包含目录
    INCLUDEPATH += $$PWD/sdk/include  # VLC 头文件路径
    INCLUDEPATH += $$PWD/sdk/include/vlc  # VLC 头文件路径

    # 设置 VLC 库文件路径
    LIBS += -L$$PWD/sdk/lib  # 指定库目录
    LIBS += -llibvlc  # 链接 VLC 库
    LIBS += -llibvlccore  # 链接 VLC 核心库
}


# 源文件和头文件
SOURCES += \
    infoform.cpp \
    loginform.cpp \
    main.cpp \
    mediabase.cpp \
    mediamp4.cpp \
    qmediaplayer.cpp \
    qscreentop.cpp \
    qstatuscheck.cpp \
    recordfile.cpp \
    ssltool.cpp \
    vlchelper.cpp \
    widget.cpp

HEADERS += \
    infoform.h \
    loginform.h \
    mediabase.h \
    mediamp4.h \
    qmediaplayer.h \
    qscreentop.h \
    qstatuscheck.h \
    recordfile.h \
    ssltool.h \
    vlchelper.h \
    widget.h

FORMS += \
    infoform.ui \
    loginForm.ui \
    messageform.ui \
    widget.ui

# 资源文件
RESOURCES += ui.qrc

# 默认部署规则
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
