#include "replacedialog.h"
#include "ui_replacedialog.h"

#include <QLineEdit>

ReplaceDialog::ReplaceDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ReplaceDialog)
{
    ui->setupUi(this);

    connect(ui->buttonFindNext, &QPushButton::clicked, this, &ReplaceDialog::requestFind);
    connect(ui->buttonReplace, &QPushButton::clicked, this, &ReplaceDialog::requestReplace);
    connect(ui->buttonReplaceAll, &QPushButton::clicked, this, &ReplaceDialog::requestReplaceAll);
    connect(ui->buttonClose, &QPushButton::clicked, this, &ReplaceDialog::close);
    connect(ui->editFind, &QLineEdit::returnPressed, this, &ReplaceDialog::requestFind);

    showStatus(tr("输入内容后单击“查找下一个”"), true);
}

ReplaceDialog::~ReplaceDialog()
{
    delete ui;
}

QString ReplaceDialog::searchText() const
{
    return ui->editFind->text();
}

void ReplaceDialog::setSearchText(const QString &text)
{
    ui->editFind->setText(text);
    ui->editFind->selectAll();
}

void ReplaceDialog::showStatus(const QString &message, bool ok)
{
    ui->labelStatus->setText(message);
    ui->labelStatus->setStyleSheet(ok ? QStringLiteral("color: #2e7d32;")
                                      : QStringLiteral("color: #c62828;"));
}

void ReplaceDialog::requestFind()
{
    emit findRequested(ui->editFind->text(),
                       ui->checkCaseSensitive->isChecked(),
                       ui->checkWholeWords->isChecked(),
                       ui->checkBackward->isChecked());
}

void ReplaceDialog::requestReplace()
{
    emit replaceRequested(ui->editFind->text(),
                          ui->editReplace->text(),
                          ui->checkCaseSensitive->isChecked(),
                          ui->checkWholeWords->isChecked());
}

void ReplaceDialog::requestReplaceAll()
{
    emit replaceAllRequested(ui->editFind->text(),
                             ui->editReplace->text(),
                             ui->checkCaseSensitive->isChecked(),
                             ui->checkWholeWords->isChecked());
}
