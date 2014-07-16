#include "sidinfo.h"
#include "ui_sidinfo.h"

SidInfo::SidInfo(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SidInfo)
{
    ui->setupUi(this);
}

SidInfo::~SidInfo()
{
    delete ui;
}

void SidInfo::name(const QString& name) {
    ui->labelName->setText(name);
}

void SidInfo::authorAndRelease(const QString& authorAndRelease) {
    ui->labelAuthorAndRelease->setText(authorAndRelease);
}
