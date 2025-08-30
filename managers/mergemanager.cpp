#include "mergemanager.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QRegularExpression>
#include <QMessageBox>
#include <QCoreApplication>
#include <QProcess>
#include <QTimer>

// 修改构造函数，初始化TableManager
MergeManager::MergeManager(TableManager* tableManager, QObject *parent)
    : QObject(parent), m_tableManager(tableManager)
{
    m_failedCount = 0;
    m_totalItems = 0;
    m_maxConcurrentProcesses = 3;
    m_exportInProgress = false;
}

// ===================== 导出接口 =====================
void MergeManager::exportItem(VideoItem* item, const QString& outputPath)
{
    startMergingProcess({item}, outputPath);
}

void MergeManager::exportSelectedItems(const QList<VideoItem*>& items, const QString& outputPath)
{
    startMergingProcess(items, outputPath);
}

void MergeManager::exportAllItems(const QList<VideoItem*>& items, const QString& outputPath)
{
    startMergingProcess(items, outputPath);
}


// ===================== 合并处理核心 =====================
void MergeManager::startMergingProcess(const QList<VideoItem*>& items, const QString& outputPath)
{
    qDebug() << "------------------ FFmpeg Merging Started ------------------";
    qDebug() << "Input Items:" << items.count();
    qDebug() << "Output Path:" << outputPath;

    m_pendingItems = items;
    m_totalItems = items.size();
    m_outputPath = outputPath;
    m_failedCount = 0;
    m_exportInProgress = true;

    emit totalProgressChanged(0);
    processNextItem();
}

void MergeManager::processNextItem()
{
    qDebug() << "MergeManager::processNextItem - Pending items:" << m_pendingItems.size();

    if (m_pendingItems.isEmpty()) {
        qDebug() << "No more items to process";
        if (m_processingItems.isEmpty()) {
            finishMergingProcess();
        }
        return;
    }

    VideoItem* item = m_pendingItems.takeFirst();
    m_processingItems.append(item);
    startFFmpegForItem(item);
}


// ===================== FFmpeg处理 =====================
void MergeManager::parseFFmpegOutput(VideoItem* item, const QString& output)
{
    // 解析FFmpeg输出获取进度值...
    int progress = extractProgress(item, output); // 传入item参数

    // 更新项目进度
    item->setProgress(progress);

    // 更新总进度
    emit totalProgressChanged(calculateTotalProgress());
}



void MergeManager::startFFmpegForItem(VideoItem* item)
{
    qDebug() << "------------------ FFmpeg Process Launched ------------------";
    qDebug() << "Video File:" << item->data(COL_VIDEO_FILE).toString();
    qDebug() << "Audio File:" << item->data(COL_AUDIO_FILE).toString();

    // 1. 获取应用程序目录
    QString appDir = QCoreApplication::applicationDirPath();

    // 2. FFmpeg路径 - 直接使用ffmpeg.exe
    QString ffmpegExe = appDir + "/ffmpeg.exe";

    // 3. 验证FFmpeg存在
    if (!QFile::exists(ffmpegExe)) {
        // ... 错误处理 ...
        emit errorOccurred("FFmpeg executable not found");
        return;
    }

    // 4. 获取视频项数据
    QString videoPath = item->data(COL_VIDEO_FILE).toString();
    QString audioPath = item->data(COL_AUDIO_FILE).toString();
    // 修改后 - 直接使用传入的m_outputPath
    QString outputPath = m_outputPath;
    QString title = item->data(COL_TITLE).toString();

    if (outputPath.isEmpty()) {
        // 只有在之前没有错误的情况下才处理
        if (!item->hasError()) {
            // 修改后 - 发送信号代替直接调用消息框
            emit errorOccurred("输出目录未设置");
            item->setProgress(-1);
            item->setHasError(true);
            m_processingItems.removeOne(item);
        }
        return;
    }

    // 5. 处理文件名中的非法字符
    QString safeTitle = title;
    QRegularExpression illegalChars(R"([\\/:*?"<>|])");
    safeTitle.replace(illegalChars, "_");

    // 6. 确保输出目录存在
    QDir outputDir(outputPath);
    if (!outputDir.exists()) {
        if (!outputDir.mkpath(".")) {
            qWarning() << "Failed to create output directory:" << outputPath;
            // 只有在之前没有处理过错误的情况下才标记失败
            if (item->progress() != -1) {
                item->setProgress(-1);
                m_processingItems.removeOne(item);
                m_failedCount++;
                // 修改后 - 发送信号代替
                emit errorOccurred("无法创建输出目录：" + outputPath);
            }
            return;
        }
    }

    if (!outputDir.exists()) {
        if (!outputDir.mkpath(".")) {
            emit errorOccurred("无法创建输出目录：" + outputPath);
            // ...错误处理...
            return;
        } else {
            emit infoMessage("已自动创建输出目录：" + outputPath);
        }
    }

    // 7. 获取输出格式
    QString format = "mp4"; // 默认MP4格式

    // 8. 构建安全的输出文件路径
    QString outputFile = outputDir.filePath(safeTitle + "." + format);

    // 9. 创建FFmpeg进程
    QProcess* ffmpegProcess = new QProcess(this);
    ffmpegProcess->setProperty("videoItem", QVariant::fromValue<VideoItem*>(item));

    // 10. 构建FFmpeg命令
    QStringList args;

    // 添加输入文件（直接使用路径）
    if (!videoPath.isEmpty()) {
        args << "-i" << videoPath;
    }
    if (!audioPath.isEmpty()) {
        args << "-i" << audioPath;
    }

    // 设置流复制参数
    args << "-c:v" << "copy" << "-c:a" << "copy";

    // 根据格式设置容器
    if (format == "mp4") {
        args << "-f" << "mp4";
    } else if (format == "mkv") {
        args << "-f" << "matroska";
    } else if (format == "webm") {
        args << "-f" << "webm";
    } else if (format == "avi") {
        args << "-f" << "avi";
    }

    // 添加输出文件参数
    args << "-y";
    args << outputFile; // 直接使用输出路径

    // 11. 连接信号处理
    connect(ffmpegProcess, &QProcess::readyReadStandardOutput, this, [this, ffmpegProcess]() {
        QString output = ffmpegProcess->readAllStandardOutput();
        VideoItem* item = ffmpegProcess->property("videoItem").value<VideoItem*>();
        if (item) parseFFmpegOutput(item, output);
    });

    connect(ffmpegProcess, &QProcess::readyReadStandardError, this, [this, ffmpegProcess]() {
        QString output = ffmpegProcess->readAllStandardError();
        VideoItem* item = ffmpegProcess->property("videoItem").value<VideoItem*>();
        if (item) parseFFmpegOutput(item, output);
    });

    // 在进程完成信号处理中添加调试输出
    connect(ffmpegProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this, ffmpegProcess](int exitCode, QProcess::ExitStatus exitStatus) {
                VideoItem* item = ffmpegProcess->property("videoItem").value<VideoItem*>();
                if (item) {
                    qDebug() << "FFmpeg进程完成，退出码:" << exitCode << "退出状态:" << exitStatus;

                    // 只有在之前没有错误的情况下才处理
                    if (!item->hasError()) {
                        if (exitStatus == QProcess::NormalExit && exitCode == 0) {
                            qDebug() << "FFmpeg处理成功";
                            item->setProgress(100);
                        } else {
                            qDebug() << "FFmpeg处理失败";
                            item->setProgress(-1);
                            item->setHasError(true);
                            m_failedCount++;
                            qDebug() << "失败计数增加，当前失败数:" << m_failedCount;

                            QString errorOutput = ffmpegProcess->readAllStandardError();
                            qDebug() << "FFmpeg错误输出:" << errorOutput;

                            // 将错误输出保存到文件
                            QFile errorLog(QCoreApplication::applicationDirPath() + "/ffmpeg_error.log");
                            if (errorLog.open(QIODevice::WriteOnly | QIODevice::Append)) {
                                errorLog.write(QString("Exit code: %1\n").arg(exitCode).toUtf8());
                                errorLog.write("Command: " + ffmpegProcess->program().toUtf8() + " " + ffmpegProcess->arguments().join(" ").toUtf8() + "\n");
                                errorLog.write("Error output:\n" + errorOutput.toUtf8() + "\n\n");
                                errorLog.close();
                            }
                        }
                    }

                    m_processingItems.removeOne(item);
                    qDebug() << "从处理队列中移除项目，当前处理中项目数:" << m_processingItems.size();
                    ffmpegProcess->deleteLater();
                    emit totalProgressChanged(calculateTotalProgress());

                    if (!m_pendingItems.isEmpty()) {
                        qDebug() << "有待处理项目，继续处理下一个";
                        processNextItem();
                    } else if (m_processingItems.isEmpty()) {
                        qDebug() << "所有项目处理完成，调用完成函数";
                        finishMergingProcess();
                    }
                }
            });

    // 在进程错误信号处理中添加调试输出
    connect(ffmpegProcess, &QProcess::errorOccurred,
            this, [this, ffmpegProcess](QProcess::ProcessError error) {
                qDebug() << "FFmpeg Error Occurred:" << error;
                VideoItem* item = ffmpegProcess->property("videoItem").value<VideoItem*>();
                qDebug() << "FFmpeg进程错误:" << error;

                // 只有在之前没有错误的情况下才处理
                if (item && !item->hasError()) {
                    // 只处理启动失败的情况，其他错误由finished信号处理
                    if (error == QProcess::FailedToStart) {
                        qDebug() << "FFmpeg启动失败";
                        item->setProgress(-1);
                        item->setHasError(true);
                        m_failedCount++;
                        qDebug() << "失败计数增加，当前失败数:" << m_failedCount;

                        QString errorStr;
                        switch(error) {
                        case QProcess::FailedToStart:
                            errorStr = "无法启动FFmpeg进程";
                            break;
                        default:
                            return;  // 其他错误类型由finished信号处理
                        }

                        qDebug() << errorStr;
                        emit errorOccurred("FFmpeg错误：" + errorStr);

                        // 保存错误信息
                        QFile errorLog(QCoreApplication::applicationDirPath() + "/ffmpeg_error.log");
                        if (errorLog.open(QIODevice::WriteOnly | QIODevice::Append)) {
                            errorLog.write(QString("Error: %1\n").arg(errorStr).toUtf8());
                            errorLog.write("Command: " + ffmpegProcess->program().toUtf8() + " " + ffmpegProcess->arguments().join(" ").toUtf8() + "\n");
                            errorLog.close();
                        }

                        m_processingItems.removeOne(item);
                        qDebug() << "从处理队列中移除项目，当前处理中项目数:" << m_processingItems.size();
                        ffmpegProcess->deleteLater();
                        emit totalProgressChanged(calculateTotalProgress());

                        if (!m_pendingItems.isEmpty()) {
                            qDebug() << "有待处理项目，继续处理下一个";
                            processNextItem();
                        } else if (m_processingItems.isEmpty()) {
                            qDebug() << "所有项目处理完成，调用完成函数";
                            finishMergingProcess();
                        }
                    }
                }
            });


    // 14. 启动进程
    qDebug() << "Executing FFmpeg command:" << ffmpegExe << args;
    ffmpegProcess->start(ffmpegExe, args);

    // 15. 添加超时处理
    QTimer::singleShot(5 * 60 * 1000, this, [ffmpegProcess, this]() {
        if (ffmpegProcess && ffmpegProcess->state() == QProcess::Running) {
            qDebug() << "FFmpeg process timed out, terminating";
            ffmpegProcess->terminate();

            // 等待5秒强制终止
            QTimer::singleShot(5000, this, [ffmpegProcess, this]() {
                if (ffmpegProcess && ffmpegProcess->state() == QProcess::Running) {
                    qDebug() << "FFmpeg process still running, killing";
                    ffmpegProcess->kill();
                }
            });
        }
    });
}


// ===================== 辅助函数 =====================
// 在文件末尾添加进度解析函数实现
int MergeManager::extractProgress(VideoItem* item, const QString& output)
{
    // 保持原有实现不变
    QRegularExpression re(R"(time=(\d+):(\d+):(\d+)\.\d+)");
    if (auto match = re.match(output); match.hasMatch()) {
        int hours = match.captured(1).toInt();
        int mins = match.captured(2).toInt();
        int secs = match.captured(3).toInt();
        int totalSecs = hours*3600 + mins*60 + secs;

        // 使用传入的item参数
        int totalDuration = item->duration();
        return totalDuration > 0 ? qMin(100, 100 * totalSecs / totalDuration) : 0;
    }
    return -1;
}


int MergeManager::calculateTotalProgress() const
{
    if (m_processingItems.isEmpty()) return 0;

    int total = 0;
    for (VideoItem* item : m_processingItems) {
        int progress = item->progress();
        if (progress < 0) progress = 0;
        if (progress > 100) progress = 100;
        total += progress;
    }
    return total / m_processingItems.size();
}


// ===================== 完成处理 =====================
void MergeManager::finishMergingProcess()
{
    qDebug() << "MergeManager::finishMergingProcess - Finishing merge process";
    m_exportInProgress = false;

    // 计算成功数量 = 总项目数 - 失败数
    int successCount = m_totalItems - m_failedCount;
    emit mergingFinished(successCount, m_failedCount);
}
