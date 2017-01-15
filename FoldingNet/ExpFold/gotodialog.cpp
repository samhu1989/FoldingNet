#include "gotodialog.h"
#include "ui_gotodialog.h"

GoToDialog::GoToDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GoToDialog)
{
    ui->setupUi(this);
}

QString GoToDialog::text(void)
{
    return ui->lineEdit->text();
}

GoToDialog::~GoToDialog()
{
    delete ui;
}
