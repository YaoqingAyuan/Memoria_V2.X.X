#ifndef MERGEMANAGER_H
#define MERGEMANAGER_H

#include <QObject>
#include <QProcess>
#include <QList>
#include "data_models/videoitem.h"
#include "data_models/tablemanager.h"  // 添加包含

class MergeManager : public QObject
{
    Q_OBJECT
public:
    explicit MergeManager(TableManager* tableManager, QObject* parent = nullptr);  // 修改构造函数

    // 导出接口
    void exportItem(VideoItem* item, const QString& outputPath);
    void exportSelectedItems(const QList<VideoItem*>& items, const QString& outputPath);
    void exportAllItems(const QList<VideoItem*>& items, const QString& outputPath);

    void startMergingProcess(const QList<VideoItem*>& items, const QString& outputPath);
    void stopMerging();

    bool isProcessing() const { return m_exportInProgress; }

signals:
    void progressChanged(int progress);
    void mergingFinished(int successCount, int failedCount);
    void errorOccurred(const QString& error);
    void itemProgressChanged(VideoItem* item, int progress);
    void totalProgressChanged(int progress);
    void infoMessage(const QString& message);

private:
    // 内部处理函数
    void processNextItem();
    void startFFmpegForItem(VideoItem* item);
    void parseFFmpegOutput(VideoItem* item, const QString& output);
    void finishMergingProcess();

    int extractProgress(VideoItem* item, const QString& output);
    int calculateTotalProgress() const;

    TableManager* m_tableManager;  // 添加TableManager指针

    // 状态变量
    QList<VideoItem*> m_processingItems;
    QList<VideoItem*> m_pendingItems;
    QString m_outputPath;
    int m_failedCount = 0;
    int m_maxConcurrentProcesses = 3;
    bool m_exportInProgress = false;
    int m_totalItems = 0;
};

#endif // MERGEMANAGER_H
