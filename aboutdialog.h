#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui { class AboutDialog; }
QT_END_NAMESPACE

/**
 * 关于对话框：显示程序版本、作者与开发环境信息
 */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
    ~AboutDialog() override;

private:
    Ui::AboutDialog *ui;
};

#endif // ABOUTDIALOG_H
