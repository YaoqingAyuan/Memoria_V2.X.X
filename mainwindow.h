#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "tablecolumns.h"
#include "videoitem.h"  // 包含VideoItem头文件
#include <QProgressBar> // 包含QProgressBar头文件

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_singleline_importButton_clicked();
    void on_wholsoueflie_importButton_clicked();
    void on_settingButton_clicked();
    void on_addlineButton_clicked();
    void on_delelineButton_clicked();

    void showContextMenu(const QPoint &pos);
    void onCustomContextMenuAction(QAction* action);

private:
    Ui::MainWindow *ui;
    void initTableView();
    void updateTableHeaders();

    ColumnManager m_columnManager;  // 列管理器实例
    QStandardItemModel* m_tableModel;  // 表格模型

    QVector<VideoItem*> m_videoItems;
    QProgressBar* m_progressDelegate;
    QList<TableColumns> m_currentColumnsOrder; // 当前列顺序

    void setupContextMenu();
    void updateRowNumbers();
    void updateTableRow(int rowIndex);
};

#endif // MAINWINDOW_H
