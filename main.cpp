#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qDebug() << "------------------ Application Startup ------------------";
    qDebug() << "Qt Version:" << QT_VERSION_STR;
    qDebug() << "Platform:" << QSysInfo::prettyProductName();

    QApplication a(argc, argv);
    qDebug() << "QApplication Initialized";

    // 配置应用程序信息（必须在创建QSettings对象前设置）
    QCoreApplication::setOrganizationName("FuliTech");
    QCoreApplication::setApplicationName("Memoria");

    MainWindow w;
    qDebug() << "MainWindow instance created."; // 调试点2：确认窗口实例创建

    w.show();
    qDebug() << "MainWindow shown."; // 调试点3：确认show()被调用

    return a.exec();
}
