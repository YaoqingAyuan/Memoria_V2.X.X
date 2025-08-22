#ifndef SINGLELINE_IMPORT_DIALOG_H
#define SINGLELINE_IMPORT_DIALOG_H

#include <QDialog>

namespace Ui {
class singleline_import_dialog;
}

class singleline_import_dialog : public QDialog
{
    Q_OBJECT

public:
    explicit singleline_import_dialog(QWidget *parent = nullptr);
    ~singleline_import_dialog();

signals:
    void importDataReady(const QString& videoPath, const QString& audioPath, const QString& title);

private slots:
    void on_VideoInputButton_clicked();
    void on_AudioInputButton_clicked();
    void on_TitlefolderButton_clicked();
    void on_OkButton_clicked();
    void on_CancelButton_clicked();

private:
    Ui::singleline_import_dialog *ui;

    bool validateM4sFile(const QString& filePath, bool isVideo);
    bool validateTitleFolder(const QString& folderPath);

    QString m_lastFileDialogPath;  // 共享的文件对话框路径
    QString m_lastTitleFolderPath;

    void loadPathSettings();
    void savePathSettings();
};

#endif // SINGLELINE_IMPORT_DIALOG_H
