#ifndef CONTEXTMENUMANAGER_H
#define CONTEXTMENUMANAGER_H

#include <QObject>
#include <QTableView>
#include <QPoint>
#include <QMenu>
#include <QAction>
#include <QDir>
#include "mainwindow.h"
#include "data_models/tablecolumns.h"  // 包含列枚举定义


class MainWindow; // 前向声明

class ContextMenuManager : public QObject
{
    Q_OBJECT
public:
    explicit ContextMenuManager(MainWindow* mainWindow, QTableView* tableView, QObject* parent = nullptr);

    void setupContextMenu();
    void showContextMenu(const QPoint& pos);

signals:
    void importFileRequested();
    void importTitleRequested();
    void importSourceRequested();

    void previewRequested(int row);
    void exportSingleRequested();
    void exportSelectedRequested();
    void exportAllRequested();

private slots:
    void onCustomContextMenuAction(QAction* action);

private:
    void loadPathSettings();
    void savePathSettings();
    void importFile(int row, int col);
    void importTitleFolder(int row);

    void createActions();
    void connectActions();

    void onPreviewAction(int row);
    void onExportSingle();
    void onExportSelected();
    void onExportAll();

    MainWindow* m_mainWindow;
    QTableView* m_tableView;
    QMenu* m_contextMenu;

    // 动作成员
    QAction* m_previewAction;
    QAction* m_deleteCurrentAction;
    QAction* m_deleteSelectedAction;
    QAction* m_deleteAllAction;
    QAction* m_importFileAction;
    QAction* m_importTitleAction;
    QAction* m_importSourceAction;
    QAction* m_exportSingleAction;
    QAction* m_exportSelectedAction;
    QAction* m_exportAllAction;

    // 路径记忆
    QString m_lastFileDialogPath;
    QString m_lastTitleFolderPath;

    // 新增辅助函数声明
    QString findMediaFile(const QDir& dir, const QString& type);
};

#endif // CONTEXTMENUMANAGER_H
