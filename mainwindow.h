#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProgressBar>
#include "data_models/tablemanager.h"
#include "delegates/deletemode.h"
#include "delegates/exportmode.h"
#include "playback_widge.h"
#include "managers/mergemanager.h" // 添加这行包含MergeManager的头文件

// 添加前向声明
class export_setting_dialog;

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

    // 公共访问方法
    DeleteMode getDeleteMode() const { return m_deleteMode; }
    bool getRememberChoice() const { return m_rememberDeleteChoice; }
    void setDeleteSettings(DeleteMode mode, bool remember);

    // 添加导出设置访问方法
    ExportMode getExportMode() const { return m_exportMode; }
    bool getExportRememberChoice() const { return m_rememberExportChoice; }
    void setExportSettings(ExportMode mode, bool remember);

    // 添加一个方法来只更新当前会话的设置而不保存到持久化设置
    void setExportSettingsSessionOnly(ExportMode mode, bool remember);

signals:
    void totalProgressChanged(int progress); // 添加总进度改变信号

private slots:
    void on_singleline_importButton_clicked();
    void on_wholsoueflie_importButton_clicked();
    void on_settingButton_clicked();
    void on_addlineButton_clicked();
    void on_delelineButton_clicked();

    void showContextMenu(const QPoint &pos);
    void onCustomContextMenuAction(QAction* action);

    void on_outputButton_clicked();

    void onExportSingle();
    void onExportSelected();
    void onExportAll();

    void on_mergeStartBtn_clicked();

    void onPreviewAction(int row);

    void handleImportData(const QString& videoPath, const QString& audioPath, const QString& title);
    void showMergeResultMessage(int successCount, int failedCount);

private:
    Ui::MainWindow *ui;

    // 添加 TableManager 成员变量
    TableManager* m_tableManager;

    QProgressBar* m_progressDelegate;

    // 删除相关成员
    DeleteMode m_deleteMode = DeleteFirst;
    bool m_rememberDeleteChoice = false;

    // 添加导出相关成员
    ExportMode m_exportMode = ExportSingle;
    bool m_rememberExportChoice = false;

    // 添加UI状态更新方法
    void updateExportStatusDisplay();

    // 执行导出操作
    void performExportOperation(ExportMode mode);

    bool m_exportInProgress = false; // 添加这个成员变量
    Playback_Widge* m_playbackWidget = nullptr; // 添加预览窗口指针

    // 路径记忆
    QString m_lastVideoPath;
    QString m_lastAudioPath;
    QString m_lastOutputPath;
    QString m_lastTitleFolderPath;

    // 初始化路径记忆
    void initPathMemory();
    // 保存路径设置
    void savePathSettings();
    // 加载路径设置
    void loadPathSettings();

    // 混流管理
    QList<VideoItem*> m_processingItems;
    QList<VideoItem*> m_pendingItems;
    int m_failedCount = 0;
    int m_maxConcurrentProcesses = 3;

    // 混流相关方法
    void startMergingProcess();
    void updateTotalProgress();

    // 删除操作管理
    void performDeleteOperation(DeleteMode mode);

    // 导出操作管理
    void exportSingleItem(int row);
    void exportSelectedItems();
    void exportAllItems();

    // 设置上下文菜单
    void setupContextMenu();

    MergeManager* m_mergeManager; // 添加 MergeManager 成员变量
};

#endif // MAINWINDOW_H
