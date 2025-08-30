#include "data_models//videoitem.h"
#include <QCryptographicHash>
#include <QFile>
#include <QDateTime>

// ===================== 构造函数 =====================
VideoItem::VideoItem(int index, QObject *parent)
    : QObject(parent), m_index(index)
{
    m_data.resize(TOTAL_COLUMNS);

    // 初始化默认值
    for (int i = 0; i < TOTAL_COLUMNS; ++i) {
        m_data[i] = "<空>";
    }

    // 设置特殊列默认值
    m_data[COL_INDEX] = index;
    m_data[COL_PROGRESS] = 0; // 进度条初始0

    m_duration = 0; // 初始化为0
}

// ===================== 数据访问函数 =====================
QVariant VideoItem::data(TableColumns column) const {
    if (column >= 0 && column < TOTAL_COLUMNS) {
        return m_data[column];
    }

    // 添加详细错误日志
    qCritical() << "VideoItem::data - 无效列索引:" << column
                << "最大允许:" << (TOTAL_COLUMNS-1);
    return QVariant();
}

void VideoItem::setData(TableColumns column, const QVariant &value) {
    if (column >= 0 && column < TOTAL_COLUMNS) {
        m_data[column] = value;
    } else {
        qCritical() << "VideoItem::setData - 无效列索引:" << column
                    << "值:" << value;
    }
}

// ===================== 特殊字段设置函数 =====================
void VideoItem::setProgress(int progress) {
    if (progress < 0) {
        progress = -1;
        m_hasError = true; // 设置错误状态
    }
    if (progress > 100) progress = 100;

    if (m_progress != progress) {
        m_progress = progress;
        m_data[COL_PROGRESS] = progress;
        emit dataChanged();
        emit progressChanged(progress);
    }
}

void VideoItem::setTitle(const QString &title) {
    m_data[COL_TITLE] = title;
    emit dataChanged();
}

void VideoItem::setIndex(int index)
{
    m_index = index;
    m_data[COL_INDEX] = index;
    emit dataChanged();
}

bool VideoItem::checkFilesExist() const
{
    QString videoPath = data(COL_VIDEO_FILE).toString();
    QString audioPath = data(COL_AUDIO_FILE).toString();

    qDebug() << "检查文件是否存在 - 视频:" << videoPath << "音频:" << audioPath;

    QFile videoFile(videoPath);
    QFile audioFile(audioPath);

    bool videoExists = videoFile.exists();
    bool audioExists = audioFile.exists();
    bool videoSizeValid = videoFile.size() > 0;
    bool audioSizeValid = audioFile.size() > 0;

    qDebug() << "文件检查结果 - 视频存在:" << videoExists << "视频大小有效:" << videoSizeValid
             << "音频存在:" << audioExists << "音频大小有效:" << audioSizeValid;

    return videoExists && audioExists && videoSizeValid && audioSizeValid;
}

QString VideoItem::generateDefaultTitle() const
{
    QString videoPath = data(COL_VIDEO_FILE).toString();
    QString audioPath = data(COL_AUDIO_FILE).toString();

    // 计算MD5哈希值
    QCryptographicHash hash(QCryptographicHash::Md5);

    QFile videoFile(videoPath);
    if (videoFile.open(QIODevice::ReadOnly)) {
        hash.addData(videoFile.read(1024)); // 只读取文件开头部分计算哈希
        videoFile.close();
    }

    QFile audioFile(audioPath);
    if (audioFile.open(QIODevice::ReadOnly)) {
        hash.addData(audioFile.read(1024));
        audioFile.close();
    }

    QString hashStr = QString(hash.result().toHex().left(8));
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");

    return QString("视频_%1_%2").arg(hashStr).arg(timestamp);
}

int VideoItem::progress() const
{
    return m_progress;
}
