#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "tablecolumns.h"


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

private:
    Ui::MainWindow *ui;
    // 添加这两个私有函数声明
    void initTableView();
    void updateTableHeaders();

    ColumnManager m_columnManager;  // 列管理器实例
    QStandardItemModel* m_tableModel;  // 表格模型

};
#endif // MAINWINDOW_H
