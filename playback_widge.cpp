#include "playback_widge.h"
#include "ui_playback_widge.h"

Playback_Widge::Playback_Widge(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Playback_Widge)
{
    ui->setupUi(this);
}

Playback_Widge::~Playback_Widge()
{
    delete ui;
}
