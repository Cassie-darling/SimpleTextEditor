#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include <QDialogButtonBox>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AboutDialog)
{
    ui->setupUi(this);

    ui->labelQt->setText(tr("开发环境：Qt %1").arg(QLatin1String(qVersion())));

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AboutDialog::accept);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}
