#ifndef MERGEMANAGER_H
#define MERGEMANAGER_H

#include <QObject>
#include <QProcess>
#include <QList>
#include "data_models/videoitem.h"

class MergeManager : public QObject
{
    Q_OBJECT
public:
    explicit MergeManager(QObject* parent = nullptr);

    void startMergingProcess(const QList<VideoItem*>& items, const QString& outputPath);
    void stopMerging();

signals:
    void progressChanged(int progress);
    void mergingFinished(int successCount, int failedCount);
    void errorOccurred(const QString& error);
    void itemProgressChanged(VideoItem* item, int progress);
    void totalProgressChanged(int progress);
    // 添加 infoMessage 信号
    void infoMessage(const QString& message);

private slots:
    void processNextItem();
    void startFFmpegForItem(VideoItem* item);
    void parseFFmpegOutput(VideoItem* item, const QString& output);
    void finishMergingProcess();

private:
    int extractProgress(const QString& output); // 声明进度解析函数

    QList<VideoItem*> m_processingItems;
    QList<VideoItem*> m_pendingItems;
    int m_failedCount = 0;
    int m_totalItems = 0; // 添加总项目数成员
    int m_maxConcurrentProcesses = 3;
    bool m_exportInProgress = false;
    QString m_outputPath;

    int calculateTotalProgress();
};

#endif // MERGEMANAGER_H
