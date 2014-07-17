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

void SidInfo::author(const QString& author) {
    ui->labelAuthor->setText(author);
}

void SidInfo::released(const QString& released) {
    ui->labelReleased->setText(released);
}
