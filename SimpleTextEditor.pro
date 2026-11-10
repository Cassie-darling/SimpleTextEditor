#-------------------------------------------------
#
# 实验2  简单文本编辑器
# 课程：Qt应用程序开发（2026年秋季）
# 作者：许龙奇
# 学号：2024414290337
# 邮箱：19310924542@163.com
#
# 说明：混合式UI设计（Designer 完成界面 + 代码完成功能），
#      使用 Action 编辑器组织菜单栏与工具栏，QPlainTextEdit 派生类实现行号。
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SimpleTextEditor
TEMPLATE = app

CONFIG += c++17

DEFINES += QT_DEPRECATED_WARNINGS

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    codeeditor.cpp \
    aboutdialog.cpp \
    finddialog.cpp \
    replacedialog.cpp

HEADERS += \
    mainwindow.h \
    codeeditor.h \
    aboutdialog.h \
    finddialog.h \
    replacedialog.h

FORMS += \
    mainwindow.ui \
    aboutdialog.ui \
    finddialog.ui \
    replacedialog.ui

RESOURCES += \
    res.qrc
