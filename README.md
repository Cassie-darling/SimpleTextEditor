# 简单文本编辑器（实验2）

Qt 应用程序开发课程（2026 年秋季）实验 2 —— 采用**混合式 UI 设计**（Qt Designer 设计界面 + 代码实现功能）的多窗口文本编辑器。

- 作者：许龙奇
- 学号：2024414290337
- 邮箱：19310924542@163.com

## 功能说明

| 分类 | 功能 |
| --- | --- |
| 文件 | 新建、新建窗口、打开、保存、另存为、退出；关闭未保存文档时提示保存；按修改状态自动更新窗口标题（`[*]`） |
| 编辑 | 撤销、重做、剪切、复制、粘贴、全选；撤销/重做、剪切/复制、粘贴按当前状态自动使能或禁用 |
| 查找替换 | 非模态“查找”与“替换”对话框；区分大小写、全词匹配、向上查找；连续查找并自动回绕；全部替换可一次撤销；未找到时给出提示 |
| 格式 | 字体与字号（QFontDialog）、字体颜色、编辑器背景色、自动换行开关、行号显示开关 |
| 视图 | 工具栏、状态栏显示/隐藏；状态栏实时显示光标行列号、字符数与修改状态 |
| 其他 | 行号由 `QPlainTextEdit` 派生类绘制（行号区宽度随行数位数自动调整）；定时自动保存（60 秒，仅对已命名且已修改的文档生效） |

## 项目结构

```
SimpleTextEditor/
├── SimpleTextEditor.pro      # qmake 工程文件
├── main.cpp                  # 程序入口
├── mainwindow.ui             # 主窗口界面（Designer 设计，Action 编辑器组织菜单与工具栏）
├── mainwindow.h / .cpp       # 主窗口逻辑：文件、编辑、格式、查找替换、状态维护
├── codeeditor.h / .cpp       # QPlainTextEdit 派生类，绘制行号与当前行高亮
├── finddialog.ui / .h / .cpp     # 查找对话框
├── replacedialog.ui / .h / .cpp  # 替换对话框
├── aboutdialog.ui / .h / .cpp    # 关于对话框
├── res.qrc                   # 资源文件
├── icons/                    # 工具栏与菜单图标（SVG）
└── .gitignore
```

## 开发与运行环境

- Qt 6.11.2（MinGW 13.1 64 位）；代码同时兼容 Qt 5.12 及以上版本
- 构建方式：qmake + MinGW Makefile（也可直接用 Qt Creator 打开 `.pro`）

```bash
# 命令行构建（Windows）
qmake SimpleTextEditor.pro
mingw32-make -j8
```

## 界面设计要点

1. 主窗口界面在 `mainwindow.ui` 中完成：中央部件为布局中的 `QPlainTextEdit`，通过“提升为（Promote）”替换为自定义 `CodeEditor` 类，并调整了布局的 `layoutMargins`。
2. 菜单栏与工具栏由 **Action 编辑器**统一创建和分组：文件、编辑、格式、视图、帮助五组 Action，工具栏中用分隔符按功能分组显示。
3. 查找、替换、关于对话框分别由独立的 `.ui` 文件设计，以非模态方式显示，便于连续操作。
