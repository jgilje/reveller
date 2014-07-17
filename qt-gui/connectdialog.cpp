#include "connectdialog.h"
#include "ui_connectdialog.h"

ConnectDialog::ConnectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectDialog)
{
    ui->setupUi(this);
}

ConnectDialog::~ConnectDialog() {
    delete ui;
}

QString ConnectDialog::url() {
    return ui->lineUrl->text();
}

void ConnectDialog::url(const QString& url) {
    ui->lineUrl->setText(url);
}
