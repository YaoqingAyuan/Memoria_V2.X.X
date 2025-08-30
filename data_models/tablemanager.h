#ifndef TABLEMANAGER_H
#define TABLEMANAGER_H

#include <QObject>
#include <QStandardItemModel>
#include <QTableView>
#include "tablecolumns.h"
#include "videoitem.h"
#include "delegates/deletemode.h"

class ProgressBarDelegate;

class TableManager : public QObject
{
    Q_OBJECT
public:
    explicit TableManager(QTableView* tableView, QObject *parent = nullptr);
    ~TableManager();

    void initTableView();
    void updateTableHeaders();
    void updateTableRow(int rowIndex);
    void updateRowNumbers();
    void clearModelData();
    void addVideoItem(const QString& videoPath, const QString& audioPath, const QString& title);
    void addNewRow(VideoItem* item);

    // 路径管理功能
    void initPathMemory();
    void loadPathSettings();
    void savePathSettings();

    // 路径访问器
    QString lastVideoPath() const { return m_lastVideoPath; }
    QString lastAudioPath() const { return m_lastAudioPath; }
    QString lastOutputPath() const { return m_lastOutputPath; }
    QString lastTitleFolderPath() const { return m_lastTitleFolderPath; }

    // 路径设置器
    void setLastVideoPath(const QString& path) { m_lastVideoPath = path; }
    void setLastAudioPath(const QString& path) { m_lastAudioPath = path; }
    void setLastOutputPath(const QString& path) { m_lastOutputPath = path; }
    void setLastTitleFolderPath(const QString& path) { m_lastTitleFolderPath = path; }

    // 其他原有方法保持不变...
    QVector<VideoItem*>& videoItems() { return m_videoItems; }
    const QVector<VideoItem*>& videoItems() const { return m_videoItems; }
    QStandardItemModel* tableModel() { return m_tableModel; }
    ColumnManager& columnManager() { return m_columnManager; }
    QList<TableColumns> currentColumnsOrder() const { return m_currentColumnsOrder; }
    void removeRow(int row);
    void removeSelectedRows(const QModelIndexList& selected);
    void removeAllRows();
    void performDeleteOperation(DeleteMode mode);
    VideoItem* videoItemAt(int row) const;
    int rowCount() const;

    void updateVideoItem(int row, TableColumns column, const QVariant& value); // 新增
    TableColumns columnTypeAt(int visualIndex) const;

private:
    QTableView* m_tableView;
    QStandardItemModel* m_tableModel;
    ColumnManager m_columnManager;
    QList<TableColumns> m_currentColumnsOrder;
    QVector<VideoItem*> m_videoItems;
    ProgressBarDelegate* m_progressDelegate;

    // 路径记忆
    QString m_lastVideoPath;
    QString m_lastAudioPath;
    QString m_lastOutputPath;
    QString m_lastTitleFolderPath;
};

#endif // TABLEMANAGER_H
