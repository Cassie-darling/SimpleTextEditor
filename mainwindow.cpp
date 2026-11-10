#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "aboutdialog.h"
#include "finddialog.h"
#include "replacedialog.h"

#include <QClipboard>
#include <QCloseEvent>
#include <QColorDialog>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDialog>
#include <QGuiApplication>
#include <QLabel>
#include <QMessageBox>
#include <QSaveFile>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTime>
#include <QTimer>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#else
#include <QTextCodec>
#endif

namespace {

// 自动保存间隔（毫秒），便于观察效果取 1 分钟
constexpr int kAutoSaveInterval = 60 * 1000;

bool isWordChar(const QChar &ch)
{
    return ch.isLetterOrNumber() || ch == QLatin1Char('_');
}

// 判断一次匹配是否满足“全词匹配”
bool isWholeWordMatch(const QTextCursor &cursor)
{
    QTextDocument *doc = cursor.document();
    const int start = cursor.selectionStart();
    const int end = cursor.selectionEnd();

    if (start > 0 && isWordChar(doc->characterAt(start - 1)))
        return false;
    if (end < doc->characterCount() && isWordChar(doc->characterAt(end)))
        return false;
    return true;
}

// 读取文件字节流并按 BOM/编码自动解码，兼容 Qt5 与 Qt6
QString decodeText(const QByteArray &data)
{
    if (data.isEmpty())
        return QString();

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const auto encoding = QStringConverter::encodingForData(data);
    QStringDecoder decoder(encoding.value_or(QStringConverter::Utf8));
    return decoder.decode(data);
#else
    QTextCodec *codec = QTextCodec::codecForUtfText(data, QTextCodec::codecForLocale());
    return codec->toUnicode(data);
#endif
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 关闭时自动释放（支持“新建窗口”打开多个编辑器窗口）
    setAttribute(Qt::WA_DeleteOnClose);

    // 状态栏信息标签
    m_positionLabel = new QLabel(this);
    m_charCountLabel = new QLabel(this);
    m_stateLabel = new QLabel(this);
    m_positionLabel->setMinimumWidth(140);
    m_charCountLabel->setMinimumWidth(110);
    m_stateLabel->setMinimumWidth(70);
    // 放在右侧常驻区域，避免被 showMessage 的临时提示覆盖
    statusBar()->addPermanentWidget(m_positionLabel);
    statusBar()->addPermanentWidget(m_charCountLabel);
    statusBar()->addPermanentWidget(m_stateLabel);

    // 查找、替换、关于对话框（非模态，便于连续操作）
    m_findDialog = new FindDialog(this);
    m_replaceDialog = new ReplaceDialog(this);
    m_aboutDialog = new AboutDialog(this);

    // ---------------- 文件 ----------------
    connect(ui->actNew, &QAction::triggered, this, &MainWindow::newFile);
    connect(ui->actNewWindow, &QAction::triggered, this, &MainWindow::newWindow);
    connect(ui->actOpen, &QAction::triggered, this, &MainWindow::openFile);
    connect(ui->actSave, &QAction::triggered, this, &MainWindow::saveFile);
    connect(ui->actSaveAs, &QAction::triggered, this, &MainWindow::saveFileAs);
    connect(ui->actExit, &QAction::triggered, this, &MainWindow::close);

    // ---------------- 编辑（Action 直接连接到编辑器槽函数） ----------------
    connect(ui->actUndo, &QAction::triggered, ui->editor, &QPlainTextEdit::undo);
    connect(ui->actRedo, &QAction::triggered, ui->editor, &QPlainTextEdit::redo);
    connect(ui->actCut, &QAction::triggered, ui->editor, &QPlainTextEdit::cut);
    connect(ui->actCopy, &QAction::triggered, ui->editor, &QPlainTextEdit::copy);
    connect(ui->actPaste, &QAction::triggered, ui->editor, &QPlainTextEdit::paste);
    connect(ui->actSelectAll, &QAction::triggered, ui->editor, &QPlainTextEdit::selectAll);

    connect(ui->actFind, &QAction::triggered, this, &MainWindow::showFindDialog);
    connect(ui->actReplace, &QAction::triggered, this, &MainWindow::showReplaceDialog);

    // ---------------- 格式 / 视图 ----------------
    connect(ui->actFont, &QAction::triggered, this, &MainWindow::changeFont);
    connect(ui->actFontColor, &QAction::triggered, this, &MainWindow::changeFontColor);
    connect(ui->actBackgroundColor, &QAction::triggered, this, &MainWindow::changeBackgroundColor);
    connect(ui->actWrap, &QAction::toggled, this, &MainWindow::toggleWordWrap);
    connect(ui->actLineNumbers, &QAction::toggled, this, &MainWindow::toggleLineNumbers);
    connect(ui->actToolBar, &QAction::toggled, this, &MainWindow::toggleToolBar);
    connect(ui->actStatusBar, &QAction::toggled, this, &MainWindow::toggleStatusBar);

    connect(ui->actAbout, &QAction::triggered, this, &MainWindow::showAboutDialog);

    // ---------------- 对话框信号 ----------------
    connect(m_findDialog, &FindDialog::findRequested, this, &MainWindow::doFind);
    connect(m_replaceDialog, &ReplaceDialog::findRequested,
            this, &MainWindow::doFindFromReplaceDialog);
    connect(m_replaceDialog, &ReplaceDialog::replaceRequested, this, &MainWindow::doReplace);
    connect(m_replaceDialog, &ReplaceDialog::replaceAllRequested, this, &MainWindow::doReplaceAll);

    // ---------------- 编辑状态 -> Action 使能 ----------------
    QTextDocument *doc = ui->editor->document();
    connect(doc, &QTextDocument::undoAvailable, ui->actUndo, &QAction::setEnabled);
    connect(doc, &QTextDocument::redoAvailable, ui->actRedo, &QAction::setEnabled);
    connect(doc, &QTextDocument::modificationChanged, this, &MainWindow::updateWindowTitle);
    connect(doc, &QTextDocument::modificationChanged, this, &MainWindow::updateStatusInfo);
    connect(ui->editor, &QPlainTextEdit::copyAvailable, this, &MainWindow::updateCopyActions);
    connect(ui->editor, &QPlainTextEdit::cursorPositionChanged, this, &MainWindow::updateStatusInfo);
    connect(ui->editor, &QPlainTextEdit::textChanged, this, &MainWindow::updateStatusInfo);

    // 剪贴板内容变化时刷新“粘贴”的使能状态
    connect(QGuiApplication::clipboard(), &QClipboard::dataChanged,
            this, &MainWindow::updatePasteAction);

    // 初始使能状态
    ui->actUndo->setEnabled(false);
    ui->actRedo->setEnabled(false);
    ui->actCut->setEnabled(false);
    ui->actCopy->setEnabled(false);
    updatePasteAction();
    updateWindowTitle();
    updateStatusInfo();

    // 定时自动保存
    m_autoSaveTimer = new QTimer(this);
    m_autoSaveTimer->setInterval(kAutoSaveInterval);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &MainWindow::autoSave);
    m_autoSaveTimer->start();

    ui->editor->setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

// ------------------------------------------------------------------
// 文件操作
// ------------------------------------------------------------------
void MainWindow::newFile()
{
    if (!maybeSave())
        return;

    m_fileName.clear();
    ui->editor->clear();
    ui->editor->document()->setModified(false);
    updateWindowTitle();
    updateStatusInfo();
    statusBar()->showMessage(tr("已新建文档"), 2000);
}

void MainWindow::newWindow()
{
    MainWindow *window = new MainWindow;
    window->show();
}

void MainWindow::openFile()
{
    if (!maybeSave())
        return;

    const QString fileName = QFileDialog::getOpenFileName(
        this, tr("打开文件"), QDir::homePath(),
        tr("文本文件 (*.txt *.cpp *.h *.md *.log *.ini);;所有文件 (*.*)"));
    if (fileName.isEmpty())
        return;

    loadFile(fileName);
}

bool MainWindow::loadFile(const QString &fileName)
{
    const QFileInfo info(fileName);
    if (!info.exists()) {
        QMessageBox::warning(this, tr("打开文件"),
                             tr("文件不存在：\n%1").arg(QDir::toNativeSeparators(fileName)));
        statusBar()->showMessage(tr("打开失败：文件不存在"), 4000);
        return false;
    }
    if (info.isDir()) {
        QMessageBox::warning(this, tr("打开文件"),
                             tr("\"%1\" 是一个目录，不能作为文本文件打开。")
                                 .arg(QDir::toNativeSeparators(fileName)));
        return false;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, tr("打开文件"),
                             tr("无法打开文件 \"%1\"：\n%2")
                                 .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        statusBar()->showMessage(tr("打开失败：%1").arg(file.errorString()), 4000);
        return false;
    }

    const QByteArray data = file.readAll();
    const QString errorString = file.errorString();
    file.close();

    if (file.error() != QFileDevice::NoError) {
        QMessageBox::warning(this, tr("打开文件"),
                             tr("读取文件 \"%1\" 时发生错误：\n%2")
                                 .arg(QDir::toNativeSeparators(fileName), errorString));
        return false;
    }

    ui->editor->setPlainText(decodeText(data));
    ui->editor->document()->setModified(false);
    m_fileName = fileName;
    updateWindowTitle();
    updateStatusInfo();
    statusBar()->showMessage(tr("已打开 %1").arg(QDir::toNativeSeparators(fileName)), 3000);
    return true;
}

bool MainWindow::saveFile()
{
    if (m_fileName.isEmpty())
        return saveFileAs();

    return writeFile(m_fileName);
}

bool MainWindow::saveFileAs()
{
    const QString fileName = QFileDialog::getSaveFileName(
        this, tr("另存为"), m_fileName.isEmpty() ? QDir::homePath() : m_fileName,
        tr("文本文件 (*.txt);;所有文件 (*.*)"));
    if (fileName.isEmpty())
        return false;

    return writeFile(fileName);
}

bool MainWindow::writeFile(const QString &fileName)
{
    QSaveFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("保存文件"),
                             tr("无法写入文件 \"%1\"：\n%2")
                                 .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    const QByteArray data = ui->editor->toPlainText().toUtf8();
    if (file.write(data) != data.size()) {
        QMessageBox::warning(this, tr("保存文件"),
                             tr("写入文件 \"%1\" 时发生错误：\n%2")
                                 .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        file.cancelWriting();
        return false;
    }

    if (!file.commit()) {
        QMessageBox::warning(this, tr("保存文件"),
                             tr("提交文件 \"%1\" 时发生错误：\n%2")
                                 .arg(QDir::toNativeSeparators(fileName), file.errorString()));
        return false;
    }

    m_fileName = fileName;
    ui->editor->document()->setModified(false);
    updateWindowTitle();
    updateStatusInfo();
    statusBar()->showMessage(tr("已保存 %1").arg(QDir::toNativeSeparators(fileName)), 3000);
    return true;
}

bool MainWindow::maybeSave()
{
    if (!ui->editor->document()->isModified())
        return true;

    const QMessageBox::StandardButton ret = QMessageBox::warning(
        this, tr("简单文本编辑器"),
        tr("文档 \"%1\" 的内容已被修改，是否保存？").arg(displayName()),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Save);

    if (ret == QMessageBox::Save)
        return saveFile();
    if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
        event->accept();
    else
        event->ignore();
}

// ------------------------------------------------------------------
// 格式设置
// ------------------------------------------------------------------
void MainWindow::changeFont()
{
    bool ok = false;
    const QFont font = QFontDialog::getFont(&ok, ui->editor->font(), this, tr("选择字体"));
    if (!ok)
        return;

    ui->editor->setFont(font);
    statusBar()->showMessage(tr("字体已设置为 %1 %2pt").arg(font.family()).arg(font.pointSize()), 3000);
}

void MainWindow::changeFontColor()
{
    QColor current = ui->editor->textCursor().charFormat().foreground().color();
    if (!current.isValid())
        current = ui->editor->palette().color(QPalette::Text);

    const QColor color = QColorDialog::getColor(current, this, tr("选择字体颜色"));
    if (!color.isValid())
        return;

    QTextCharFormat format = ui->editor->currentCharFormat();
    format.setForeground(color);
    // 有选中内容时作用于选中部分，否则作用于之后输入的文本
    ui->editor->mergeCurrentCharFormat(format);
}

void MainWindow::changeBackgroundColor()
{
    const QColor color = QColorDialog::getColor(ui->editor->palette().color(QPalette::Base),
                                                this, tr("选择编辑器背景颜色"));
    if (!color.isValid())
        return;

    QPalette palette = ui->editor->palette();
    palette.setColor(QPalette::Base, color);
    ui->editor->setPalette(palette);
    ui->editor->viewport()->update();
}

void MainWindow::toggleWordWrap(bool enabled)
{
    ui->editor->setLineWrapMode(enabled ? QPlainTextEdit::WidgetWidth
                                        : QPlainTextEdit::NoWrap);
    statusBar()->showMessage(enabled ? tr("已开启自动换行") : tr("已关闭自动换行"), 2000);
}

void MainWindow::toggleLineNumbers(bool enabled)
{
    ui->editor->setLineNumbersVisible(enabled);
    statusBar()->showMessage(enabled ? tr("已显示行号") : tr("已隐藏行号"), 2000);
}

// ------------------------------------------------------------------
// 视图
// ------------------------------------------------------------------
void MainWindow::toggleToolBar(bool visible)
{
    ui->mainToolBar->setVisible(visible);
}

void MainWindow::toggleStatusBar(bool visible)
{
    statusBar()->setVisible(visible);
}

// ------------------------------------------------------------------
// 对话框
// ------------------------------------------------------------------
void MainWindow::showFindDialog()
{
    const QTextCursor cursor = ui->editor->textCursor();
    if (cursor.hasSelection()) {
        QString text = cursor.selectedText();
        text.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
        if (!text.contains(QLatin1Char('\n')))
            m_findDialog->setSearchText(text);
    }

    m_findDialog->show();
    m_findDialog->raise();
    m_findDialog->activateWindow();
}

void MainWindow::showReplaceDialog()
{
    const QTextCursor cursor = ui->editor->textCursor();
    if (cursor.hasSelection()) {
        QString text = cursor.selectedText();
        text.replace(QChar::ParagraphSeparator, QLatin1Char('\n'));
        if (!text.contains(QLatin1Char('\n')))
            m_replaceDialog->setSearchText(text);
    }

    m_replaceDialog->show();
    m_replaceDialog->raise();
    m_replaceDialog->activateWindow();
}

void MainWindow::showAboutDialog()
{
    m_aboutDialog->show();
    m_aboutDialog->raise();
    m_aboutDialog->activateWindow();
}

// ------------------------------------------------------------------
// 查找 / 替换
// ------------------------------------------------------------------
QTextCursor MainWindow::searchFrom(const QTextCursor &from, const QString &text,
                                  bool caseSensitive, bool wholeWords, bool backward)
{
    QTextDocument::FindFlags flags;
    if (caseSensitive)
        flags |= QTextDocument::FindCaseSensitively;
    if (backward)
        flags |= QTextDocument::FindBackward;

    QTextDocument *doc = ui->editor->document();
    QTextCursor cursor = from;

    while (true) {
        const QTextCursor found = doc->find(text, cursor, flags);
        if (found.isNull())
            return QTextCursor();
        if (!wholeWords || isWholeWordMatch(found))
            return found;
        // 词内匹配时跳过本次，继续查找
        cursor = found;
    }
}

bool MainWindow::findNext(const QString &text, bool caseSensitive, bool wholeWords,
                          bool backward, bool wrap)
{
    QTextCursor found = searchFrom(ui->editor->textCursor(), text,
                                   caseSensitive, wholeWords, backward);

    if (found.isNull() && wrap) {
        // 到达文档头/尾后从头（或尾）继续查找，实现连续查找
        QTextCursor edge(ui->editor->document());
        edge.movePosition(backward ? QTextCursor::End : QTextCursor::Start);
        found = searchFrom(edge, text, caseSensitive, wholeWords, backward);
    }

    if (found.isNull())
        return false;

    ui->editor->setTextCursor(found);
    ui->editor->ensureCursorVisible();
    return true;
}

void MainWindow::doFind(const QString &text, bool caseSensitive, bool wholeWords, bool backward)
{
    if (text.isEmpty()) {
        m_findDialog->showStatus(tr("请先输入要查找的内容"), false);
        return;
    }

    if (findNext(text, caseSensitive, wholeWords, backward, true))
        m_findDialog->showStatus(tr("已找到匹配内容"), true);
    else
        m_findDialog->showStatus(tr("未找到 \"%1\"").arg(text), false);
}

void MainWindow::doFindFromReplaceDialog(const QString &text, bool caseSensitive,
                                        bool wholeWords, bool backward)
{
    if (text.isEmpty()) {
        m_replaceDialog->showStatus(tr("请先输入要查找的内容"), false);
        return;
    }

    if (findNext(text, caseSensitive, wholeWords, backward, true))
        m_replaceDialog->showStatus(tr("已找到匹配内容"), true);
    else
        m_replaceDialog->showStatus(tr("未找到 \"%1\"").arg(text), false);
}

void MainWindow::doReplace(const QString &text, const QString &replacement,
                           bool caseSensitive, bool wholeWords)
{
    if (text.isEmpty()) {
        m_replaceDialog->showStatus(tr("请先输入要查找的内容"), false);
        return;
    }

    QTextCursor cursor = ui->editor->textCursor();
    bool replaced = false;

    if (cursor.hasSelection()) {
        const QString selected = cursor.selectedText();
        const bool same = caseSensitive
                              ? (selected == text)
                              : (selected.compare(text, Qt::CaseInsensitive) == 0);
        if (same) {
            cursor.insertText(replacement);
            replaced = true;
        }
    }

    if (findNext(text, caseSensitive, wholeWords, false, true)) {
        m_replaceDialog->showStatus(replaced ? tr("已替换 1 处，并定位到下一处")
                                             : tr("已找到匹配内容"), true);
    } else {
        m_replaceDialog->showStatus(replaced ? tr("已替换 1 处，未找到更多匹配")
                                             : tr("未找到 \"%1\"").arg(text), replaced);
    }
}

void MainWindow::doReplaceAll(const QString &text, const QString &replacement,
                              bool caseSensitive, bool wholeWords)
{
    if (text.isEmpty()) {
        m_replaceDialog->showStatus(tr("请先输入要查找的内容"), false);
        return;
    }

    QTextDocument *doc = ui->editor->document();
    QTextCursor cursor(doc);
    cursor.movePosition(QTextCursor::Start);

    // 放在同一个编辑块中，保证“全部替换”可以一次撤销
    cursor.beginEditBlock();

    int count = 0;
    QTextCursor found = searchFrom(cursor, text, caseSensitive, wholeWords, false);
    while (!found.isNull()) {
        found.insertText(replacement);
        ++count;
        found = searchFrom(found, text, caseSensitive, wholeWords, false);
    }

    cursor.endEditBlock();

    if (count > 0)
        m_replaceDialog->showStatus(tr("共替换 %1 处").arg(count), true);
    else
        m_replaceDialog->showStatus(tr("未找到 \"%1\"").arg(text), false);
}

// ------------------------------------------------------------------
// 状态维护
// ------------------------------------------------------------------
void MainWindow::autoSave()
{
    if (m_fileName.isEmpty() || !ui->editor->document()->isModified())
        return;

    if (writeFile(m_fileName))
        statusBar()->showMessage(
            tr("已自动保存（%1）").arg(QTime::currentTime().toString(QStringLiteral("hh:mm:ss"))),
            3000);
}

void MainWindow::updateWindowTitle()
{
    setWindowTitle(tr("%1[*] - 简单文本编辑器").arg(displayName()));
    setWindowModified(ui->editor->document()->isModified());
}

void MainWindow::updateStatusInfo()
{
    const QTextCursor cursor = ui->editor->textCursor();
    m_positionLabel->setText(tr("行 %1，列 %2")
                                 .arg(cursor.blockNumber() + 1)
                                 .arg(cursor.positionInBlock() + 1));
    m_charCountLabel->setText(tr("字符数：%1").arg(ui->editor->document()->characterCount() - 1));
    m_stateLabel->setText(ui->editor->document()->isModified() ? tr("已修改") : tr("就绪"));
}

void MainWindow::updateCopyActions(bool hasSelection)
{
    ui->actCut->setEnabled(hasSelection);
    ui->actCopy->setEnabled(hasSelection);
}

void MainWindow::updatePasteAction()
{
    ui->actPaste->setEnabled(ui->editor->canPaste());
}

QString MainWindow::displayName() const
{
    return m_fileName.isEmpty() ? tr("未命名") : QFileInfo(m_fileName).fileName();
}
