#ifndef REPLACEDIALOG_H
#define REPLACEDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class ReplaceDialog; }
QT_END_NAMESPACE

/**
 * 替换对话框：支持查找下一个、替换、全部替换
 */
class ReplaceDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReplaceDialog(QWidget *parent = nullptr);
    ~ReplaceDialog() override;

    QString searchText() const;
    void setSearchText(const QString &text);
    void showStatus(const QString &message, bool ok);

signals:
    void findRequested(const QString &text, bool caseSensitive, bool wholeWords, bool backward);
    void replaceRequested(const QString &text, const QString &replacement,
                          bool caseSensitive, bool wholeWords);
    void replaceAllRequested(const QString &text, const QString &replacement,
                             bool caseSensitive, bool wholeWords);

private slots:
    void requestFind();
    void requestReplace();
    void requestReplaceAll();

private:
    Ui::ReplaceDialog *ui;
};

#endif // REPLACEDIALOG_H
