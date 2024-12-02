QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# 添加 OpenSSL 的路径
INCLUDEPATH += C:/src/vcpkg/installed/x64-windows-static/include
LIBS += -LC:/src/vcpkg/installed/x64-windows-static/lib
LIBS += -llibcrypto -llibssl -lws2_32 -lbcrypt -lgdi32
LIBS += -lcrypt32 -luser32 -lkernel32 -ladvapi32


# 源文件和头文件
SOURCES += \
    infoform.cpp \
    loginform.cpp \
    main.cpp \
    recordfile.cpp \
    ssltool.cpp \
    widget.cpp

HEADERS += \
    infoform.h \
    loginform.h \
    recordfile.h \
    ssltool.h \
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
