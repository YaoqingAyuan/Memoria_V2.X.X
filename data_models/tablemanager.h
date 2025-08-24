#ifndef TABLEMANAGER_H
#define TABLEMANAGER_H

#include <QObject>
#include <QStandardItemModel>
#include <QTableView>
#include "tablecolumns.h"
#include "videoitem.h"

// 前向声明
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
    void addNewRow(VideoItem* item);

    // 获取视频项列表
    QVector<VideoItem*>& videoItems() { return m_videoItems; }
    const QVector<VideoItem*>& videoItems() const { return m_videoItems; }

    // 获取表格模型
    QStandardItemModel* tableModel() { return m_tableModel; }

    // 获取列管理器
    ColumnManager& columnManager() { return m_columnManager; }

    // 获取当前列顺序
    QList<TableColumns> currentColumnsOrder() const { return m_currentColumnsOrder; }

    // 添加这些方法
    void removeRow(int row);
    void removeSelectedRows(const QModelIndexList& selected);
    void removeAllRows();

    // 获取特定行的视频项
    VideoItem* videoItemAt(int row) const;

    // 获取行数
    int rowCount() const;

private:
    QTableView* m_tableView;
    QStandardItemModel* m_tableModel;
    ColumnManager m_columnManager;
    QList<TableColumns> m_currentColumnsOrder;
    QVector<VideoItem*> m_videoItems;
    ProgressBarDelegate* m_progressDelegate;
};

#endif // TABLEMANAGER_H
