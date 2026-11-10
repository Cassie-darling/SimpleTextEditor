#ifndef FINDDIALOG_H
#define FINDDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class FindDialog; }
QT_END_NAMESPACE

/**
 * 查找对话框：非模态，便于连续查找
 */
class FindDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FindDialog(QWidget *parent = nullptr);
    ~FindDialog() override;

    QString searchText() const;
    void setSearchText(const QString &text);
    // 在对话框中显示操作结果（ok 为 false 时以红色提示“未找到”等）
    void showStatus(const QString &message, bool ok);

signals:
    void findRequested(const QString &text, bool caseSensitive, bool wholeWords, bool backward);

private slots:
    void requestFind();

private:
    Ui::FindDialog *ui;
};

#endif // FINDDIALOG_H
