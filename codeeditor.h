#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>

class LineNumberArea;

/**
 * 支持行号显示的文本编辑器（参考 Qt 官方 Code Editor 示例）
 * 通过 ui 文件中的“提升(Promote)”操作替换为 QPlainTextEdit 的派生类
 */
class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent = nullptr);

    // 由行号区域调用，完成行号绘制
    void lineNumberAreaPaintEvent(QPaintEvent *event);
    // 行号区域的推荐宽度（随行数位数变化）
    int lineNumberAreaWidth();

    void setLineNumbersVisible(bool visible);
    bool lineNumbersVisible() const;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void changeEvent(QEvent *event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    LineNumberArea *m_lineNumberArea;
};

#endif // CODEEDITOR_H
