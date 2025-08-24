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
     explicit MergeManager(QObject* parent = nullptr); // 修正构造函数声明

    void startMergingProcess(const QList<VideoItem*>& items, const QString& outputPath);
    void stopMerging();

signals:
    void progressChanged(int progress);
    void mergingFinished(int successCount, int failedCount);
    void errorOccurred(const QString& error);
    void itemProgressChanged(VideoItem* item, int progress);
    void totalProgressChanged(int progress);

private slots:
    void processNextItem();
    void startFFmpegForItem(VideoItem* item);
    void parseFFmpegOutput(VideoItem* item, const QString& output);
    void finishMergingProcess();

private:
    QList<VideoItem*> m_processingItems;
    QList<VideoItem*> m_pendingItems;
    int m_failedCount = 0;
    int m_maxConcurrentProcesses = 3;
    bool m_exportInProgress = false;
    QString m_outputPath;
    int calculateTotalProgress(); // 添加这行

};

#endif // MERGEMANAGER_H
