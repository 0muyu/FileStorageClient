# 1) 记录本模块根目录
MEDIAPLAYER_ROOT = $$PWD

# 2) 头文件搜索路径
INCLUDEPATH += \
    $$MEDIAPLAYER_ROOT \
    $$MEDIAPLAYER_ROOT/ffmpeg-4.2.2/include \
    $$MEDIAPLAYER_ROOT/SDL2-2.0.10/include \
    $$MEDIAPLAYER_ROOT/opengl

# 3) 库文件搜索路径（根据实际编译器/平台修改）
#    这里给出 Windows MSVC 示例；Linux/Mac 换成 .so/.dylib 路径
win32:CONFIG(release, debug|release) {
    LIBS += -L$$MEDIAPLAYER_ROOT/ffmpeg-4.2.2/lib \
            -lavcodec -lavformat -lavutil -lswscale -lswresample -lavdevice
    LIBS += -L$$MEDIAPLAYER_ROOT/SDL2-2.0.10/lib/x86 \
            -lSDL2
} else:win32:CONFIG(debug, debug|release) {
    LIBS += -L$$MEDIAPLAYER_ROOT/ffmpeg-4.2.2/lib \
            -lavcodec -lavformat -lavutil -lswscale -lswresample -lavdevice
    LIBS += -L$$MEDIAPLAYER_ROOT/SDL2-2.0.10/lib/x86 \
            -lSDL2
}

# 4) 源文件（排除了 main.cpp）
SOURCES += \
    $$MEDIAPLAYER_ROOT/packetqueue.cpp \
    $$MEDIAPLAYER_ROOT/playerdialog.cpp \
    $$MEDIAPLAYER_ROOT/videoplayer.cpp

HEADERS += \
    $$MEDIAPLAYER_ROOT/packetqueue.h \
    $$MEDIAPLAYER_ROOT/playerdialog.h \
    $$MEDIAPLAYER_ROOT/videoplayer.h

FORMS += \
    $$MEDIAPLAYER_ROOT/playerdialog.ui

# 5) 引入子模块
include($$MEDIAPLAYER_ROOT/opengl/opengl.pri)

# 6) Qt 模块
QT += widgets multimedia opengl

# 7) 运行时把 dll 拷到生成目录（仅 Windows）
win32 {
    DLL_SRC = $$MEDIAPLAYER_ROOT/dll
    DLL_DST = $$OUT_PWD/debug
    CONFIG(release, debug|release): DLL_DST = $$OUT_PWD/release
    QMAKE_POST_LINK += $$quote(cmd /c xcopy "$$replace(DLL_SRC,/,\\)*.dll" "$$replace(DLL_DST,/,\\)" /Y /D)
}
