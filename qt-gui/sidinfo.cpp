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

void SidInfo::setHeader(const SidHeader &header) {
    _header = header;

    ui->labelName->setText(header.name);
    ui->labelAuthor->setText(header.author);
    ui->labelReleased->setText(header.released);

    // header.startSong
}
