#include "dialogs/singleline_import_dialog.h"
#include "dialogs/ui_singleline_import_dialog.h"
#include <QSettings>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QProcess>

// ===================== 构造函数/析构函数 =====================
singleline_import_dialog::singleline_import_dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::singleline_import_dialog)
{
    ui->setupUi(this);

    // 加载路径设置
    loadPathSettings();
}

singleline_import_dialog::~singleline_import_dialog()
{
    delete ui;
}

// ===================== 验证M4S文件 =====================
bool singleline_import_dialog::validateM4sFile(const QString& filePath, bool isVideo)
{
    if (!QFile::exists(filePath)) {
        QMessageBox::warning(this, "错误", "文件不存在");
        return false;
    }

    if (!filePath.toLower().endsWith(".m4s")) {
        QMessageBox::warning(this, "错误", "请选择.m4s文件");
        return false;
    }

    // 简单的格式检查
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly)) {
        // 读取文件头，检查是否是有效的媒体文件
        QByteArray header = file.read(8);
        file.close();

        // MP4 文件通常以 "ftyp" 开头
        if (header.size() >= 8 && header.contains("ftyp")) {
            return true; // 看起来是有效的 MP4 文件，跳过详细验证
        }
    }

    // 使用FFmpeg验证文件类型
    QProcess ffmpeg;
    ffmpeg.start("ffmpeg", QStringList() << "-i" << filePath);

    if (!ffmpeg.waitForStarted()) {
        // FFmpeg不可用，跳过详细验证
        QMessageBox::warning(this, "警告", "无法启动FFmpeg进行文件验证，将跳过详细检查");
        return true;
    }

    // 等待进程完成，设置超时时间
    if (!ffmpeg.waitForFinished(5000)) { // 5秒超时
        QMessageBox::warning(this, "警告", "FFmpeg验证超时，将跳过详细检查");
        ffmpeg.kill(); // 终止进程
        return true;
    }

    QString output = ffmpeg.readAllStandardError();

    // 检查中文和英文的输出
    if (isVideo) {
        if (!output.contains("Video:") && !output.contains("视频:")) {
            QMessageBox::warning(this, "错误", "选择的文件不包含视频流");
            return false;
        }
    } else {
        if (!output.contains("Audio:") && !output.contains("音频:")) {
            QMessageBox::warning(this, "错误", "选择的文件不包含音频流");
            return false;
        }
    }

    return true;
}

// ===================== 验证标题文件夹 =====================
bool singleline_import_dialog::validateTitleFolder(const QString& folderPath)
{
    QDir dir(folderPath);
    if (!dir.exists()) {
        QMessageBox::warning(this, "错误", "文件夹不存在");
        return false;
    }

    // 检查文件夹中是否有video.m4s和audio.m4s文件
    bool hasVideo = QFile::exists(dir.filePath("video.m4s"));
    bool hasAudio = QFile::exists(dir.filePath("audio.m4s"));

    if (!hasVideo && !hasAudio) {
        QMessageBox::warning(this, "错误", "文件夹中未找到video.m4s或audio.m4s文件");
        return false;
    }

    return true;
}


// ===================== 按钮槽函数 =====================
// 视频按钮槽函数
void singleline_import_dialog::on_VideoInputButton_clicked()
{
    QString videoPath = QFileDialog::getOpenFileName(
        this,
        tr("选择视频文件"),
        m_lastFileDialogPath,  // 使用共享的文件对话框路径
        tr("M4S文件 (*.m4s);;所有文件 (*)")
        );

    if (!videoPath.isEmpty() && validateM4sFile(videoPath, true)) {
        ui->VideoAddline->setText(videoPath);

        // 更新共享的文件对话框路径
        m_lastFileDialogPath = QFileInfo(videoPath).absolutePath();
        savePathSettings();
    }
}

// 音频按钮槽函数
void singleline_import_dialog::on_AudioInputButton_clicked()
{
    QString audioPath = QFileDialog::getOpenFileName(
        this,
        tr("选择音频文件"),
        m_lastFileDialogPath,  // 使用共享的文件对话框路径
        tr("M4S文件 (*.m4s);;所有文件 (*)")
        );

    if (!audioPath.isEmpty() && validateM4sFile(audioPath, false)) {
        ui->AudioAddline->setText(audioPath);

        // 更新共享的文件对话框路径
        m_lastFileDialogPath = QFileInfo(audioPath).absolutePath();
        savePathSettings();
    }
}

// 标题文件夹导入模式槽函数
void singleline_import_dialog::on_TitlefolderButton_clicked()
{
    // 获取初始路径：如果是第一次使用，使用默认路径；否则使用上次选择的文件夹的父目录
    QString initialPath;
    if (m_lastTitleFolderPath.isEmpty() || !QDir(m_lastTitleFolderPath).exists()) {
        initialPath = QDir::homePath();
    } else {
        // 使用上次选择文件夹的父目录
        QDir lastDir(m_lastTitleFolderPath);
        if (lastDir.cdUp()) {
            initialPath = lastDir.path();
        } else {
            initialPath = QDir::homePath();
        }
    }

    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        tr("选择标题文件夹"),
        initialPath,  // 使用父目录作为初始路径
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (!folderPath.isEmpty() && validateTitleFolder(folderPath)) {
        QDir dir(folderPath);

        // 设置标题为文件夹名称
        ui->TitlelineEdit->setText(dir.dirName());

        // 自动填充视频和音频路径
        QString videoPath = dir.filePath("video.m4s");
        if (QFile::exists(videoPath)) {
            ui->VideoAddline->setText(videoPath);
        }

        QString audioPath = dir.filePath("audio.m4s");
        if (QFile::exists(audioPath)) {
            ui->AudioAddline->setText(audioPath);
        }

        // 更新路径记忆 - 存储实际选择的文件夹路径
        m_lastTitleFolderPath = folderPath;
        savePathSettings();

        // 同时更新文件对话框路径，以便视频和音频浏览按钮使用
        m_lastFileDialogPath = folderPath;
    }
}

void singleline_import_dialog::on_OkButton_clicked()
{
    QString videoPath = ui->VideoAddline->text();
    QString audioPath = ui->AudioAddline->text();
    QString title = ui->TitlelineEdit->text();

    // 验证必要字段
    if (title.isEmpty()) {
        QMessageBox::warning(this, "错误", "请输入视频标题");
        return;
    }

    if (videoPath.isEmpty() && audioPath.isEmpty()) {
        QMessageBox::warning(this, "错误", "请至少提供视频或音频文件");
        return;
    }

    // 验证文件是否存在
    if (!videoPath.isEmpty() && !QFile::exists(videoPath)) {
        QMessageBox::warning(this, "错误", "视频文件不存在");
        return;
    }

    if (!audioPath.isEmpty() && !QFile::exists(audioPath)) {
        QMessageBox::warning(this, "错误", "音频文件不存在");
        return;
    }

    // 在发送数据前，将共享路径回退到上一级目录
    if (!m_lastFileDialogPath.isEmpty() && QDir(m_lastFileDialogPath).exists()) {
        QDir dir(m_lastFileDialogPath);
        if (dir.cdUp()) {
            m_lastFileDialogPath = dir.path();
            savePathSettings();
        }
    }

    // 发送数据到主窗口
    emit importDataReady(videoPath, audioPath, title);
    accept();
}


void singleline_import_dialog::on_CancelButton_clicked()
{
    reject();
}

void singleline_import_dialog::loadPathSettings()
{
    QSettings settings;
    m_lastFileDialogPath = settings.value("Last/FileDialogPath", QDir::homePath()).toString();
    m_lastTitleFolderPath = settings.value("Last/TitleFolderPath", QDir::homePath()).toString();
}

void singleline_import_dialog::savePathSettings()
{
    QSettings settings;
    settings.setValue("Last/FileDialogPath", m_lastFileDialogPath);
    settings.setValue("Last/TitleFolderPath", m_lastTitleFolderPath);
}
