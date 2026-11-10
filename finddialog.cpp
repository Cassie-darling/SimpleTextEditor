#include "finddialog.h"
#include "ui_finddialog.h"

#include <QLineEdit>

FindDialog::FindDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FindDialog)
{
    ui->setupUi(this);

    connect(ui->buttonFindNext, &QPushButton::clicked, this, &FindDialog::requestFind);
    connect(ui->buttonClose, &QPushButton::clicked, this, &FindDialog::close);
    connect(ui->editFind, &QLineEdit::returnPressed, this, &FindDialog::requestFind);

    showStatus(tr("输入内容后单击“查找下一个”"), true);
}

FindDialog::~FindDialog()
{
    delete ui;
}

QString FindDialog::searchText() const
{
    return ui->editFind->text();
}

void FindDialog::setSearchText(const QString &text)
{
    ui->editFind->setText(text);
    ui->editFind->selectAll();
}

void FindDialog::showStatus(const QString &message, bool ok)
{
    ui->labelStatus->setText(message);
    ui->labelStatus->setStyleSheet(ok ? QStringLiteral("color: #2e7d32;")
                                      : QStringLiteral("color: #c62828;"));
}

void FindDialog::requestFind()
{
    const QString text = ui->editFind->text();
    emit findRequested(text,
                       ui->checkCaseSensitive->isChecked(),
                       ui->checkWholeWords->isChecked(),
                       ui->checkBackward->isChecked());
}
