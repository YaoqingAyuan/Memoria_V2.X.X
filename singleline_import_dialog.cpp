#include "singleline_import_dialog.h"
#include "ui_singleline_import_dialog.h"
#include <QSettings>

singleline_import_dialog::singleline_import_dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::singleline_import_dialog)
{
    ui->setupUi(this);
}

singleline_import_dialog::~singleline_import_dialog()
{
    delete ui;
}

//对应两个“浏览按钮”的槽函数，点击后可浏览地址（MemoriaV1.X.X照搬都可以）
//视频按钮槽函数
void singleline_import_dialog::on_VideoInputButton_clicked()
{

}

//音频按钮槽函数
void singleline_import_dialog::on_AudioInputButton_clicked()
{

}

//标题文件夹导入模式槽函数
void singleline_import_dialog::on_TitlefolderButton_clicked()
{

}

