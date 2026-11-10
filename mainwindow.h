#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextCursor>
#include <QTextDocument>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class AboutDialog;
class FindDialog;
class ReplaceDialog;
class QCloseEvent;
class QLabel;
class QTimer;

/**
 * 主窗口：简单文本编辑器
 * 界面由 mainwindow.ui（Qt Designer）设计，工具栏/菜单栏由 Action 编辑器组织，
 * 功能逻辑在本类中实现。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // 文件操作
    void newFile();
    void newWindow();
    void openFile();
    bool saveFile();
    bool saveFileAs();

    // 格式设置
    void changeFont();
    void changeFontColor();
    void changeBackgroundColor();
    void toggleWordWrap(bool enabled);
    void toggleLineNumbers(bool enabled);

    // 视图
    void toggleToolBar(bool visible);
    void toggleStatusBar(bool visible);

    // 对话框
    void showFindDialog();
    void showReplaceDialog();
    void showAboutDialog();

    // 查找 / 替换
    void doFind(const QString &text, bool caseSensitive, bool wholeWords, bool backward);
    void doFindFromReplaceDialog(const QString &text, bool caseSensitive,
                                 bool wholeWords, bool backward);
    void doReplace(const QString &text, const QString &replacement,
                   bool caseSensitive, bool wholeWords);
    void doReplaceAll(const QString &text, const QString &replacement,
                      bool caseSensitive, bool wholeWords);

    // 状态维护
    void autoSave();
    void updateWindowTitle();
    void updateStatusInfo();
    void updateCopyActions(bool hasSelection);
    void updatePasteAction();

private:
    bool maybeSave();
    bool loadFile(const QString &fileName);
    bool writeFile(const QString &fileName);
    QString displayName() const;

    bool findNext(const QString &text, bool caseSensitive, bool wholeWords,
                  bool backward, bool wrap);
    QTextCursor searchFrom(const QTextCursor &from, const QString &text,
                           bool caseSensitive, bool wholeWords, bool backward);

    Ui::MainWindow *ui;
    QString m_fileName;
    FindDialog *m_findDialog = nullptr;
    ReplaceDialog *m_replaceDialog = nullptr;
    AboutDialog *m_aboutDialog = nullptr;
    QLabel *m_positionLabel = nullptr;
    QLabel *m_charCountLabel = nullptr;
    QLabel *m_stateLabel = nullptr;
    QTimer *m_autoSaveTimer = nullptr;
};

#endif // MAINWINDOW_H
