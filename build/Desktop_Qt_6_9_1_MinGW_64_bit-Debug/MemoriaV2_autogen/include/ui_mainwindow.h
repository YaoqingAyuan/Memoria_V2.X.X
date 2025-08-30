/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QWidget *centralwidget;
    QPushButton *singleline_importButton;
    QPushButton *wholsoueflie_importButton;
    QPushButton *settingButton;
    QPushButton *addlineButton;
    QPushButton *delelineButton;
    QLabel *label;
    QLineEdit *outputAdd_Edit;
    QPushButton *outputButton;
    QPushButton *mergeStartBtn;
    QWidget *widget;
    QVBoxLayout *verticalLayout;
    QProgressBar *Total_progressBar;
    QTableView *MaintableView;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(800, 600);
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        singleline_importButton = new QPushButton(centralwidget);
        singleline_importButton->setObjectName("singleline_importButton");
        singleline_importButton->setGeometry(QRect(60, 110, 121, 21));
        wholsoueflie_importButton = new QPushButton(centralwidget);
        wholsoueflie_importButton->setObjectName("wholsoueflie_importButton");
        wholsoueflie_importButton->setGeometry(QRect(60, 150, 121, 21));
        settingButton = new QPushButton(centralwidget);
        settingButton->setObjectName("settingButton");
        settingButton->setGeometry(QRect(680, 150, 80, 21));
        addlineButton = new QPushButton(centralwidget);
        addlineButton->setObjectName("addlineButton");
        addlineButton->setGeometry(QRect(120, 70, 31, 21));
        QFont font;
        font.setPointSize(16);
        addlineButton->setFont(font);
        delelineButton = new QPushButton(centralwidget);
        delelineButton->setObjectName("delelineButton");
        delelineButton->setGeometry(QRect(150, 70, 31, 21));
        delelineButton->setFont(font);
        label = new QLabel(centralwidget);
        label->setObjectName("label");
        label->setGeometry(QRect(270, 110, 51, 21));
        QFont font1;
        font1.setFamilies({QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221")});
        font1.setPointSize(10);
        label->setFont(font1);
        outputAdd_Edit = new QLineEdit(centralwidget);
        outputAdd_Edit->setObjectName("outputAdd_Edit");
        outputAdd_Edit->setGeometry(QRect(320, 110, 211, 22));
        outputButton = new QPushButton(centralwidget);
        outputButton->setObjectName("outputButton");
        outputButton->setGeometry(QRect(530, 110, 80, 21));
        mergeStartBtn = new QPushButton(centralwidget);
        mergeStartBtn->setObjectName("mergeStartBtn");
        mergeStartBtn->setGeometry(QRect(370, 160, 131, 31));
        QFont font2;
        font2.setPointSize(12);
        mergeStartBtn->setFont(font2);
        widget = new QWidget(centralwidget);
        widget->setObjectName("widget");
        widget->setGeometry(QRect(-10, 230, 811, 351));
        verticalLayout = new QVBoxLayout(widget);
        verticalLayout->setObjectName("verticalLayout");
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        Total_progressBar = new QProgressBar(widget);
        Total_progressBar->setObjectName("Total_progressBar");
        Total_progressBar->setValue(24);

        verticalLayout->addWidget(Total_progressBar);

        MaintableView = new QTableView(widget);
        MaintableView->setObjectName("MaintableView");
        MaintableView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);

        verticalLayout->addWidget(MaintableView);

        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 800, 19));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        singleline_importButton->setText(QCoreApplication::translate("MainWindow", "\345\257\274\345\205\245\345\215\225\344\270\252\350\247\206\351\242\221\342\200\246", nullptr));
        wholsoueflie_importButton->setText(QCoreApplication::translate("MainWindow", "\345\257\274\345\205\245\346\225\264\344\270\252\347\274\223\345\255\230\346\272\220\346\226\207\344\273\266", nullptr));
        settingButton->setText(QCoreApplication::translate("MainWindow", "\350\256\276\347\275\256", nullptr));
        addlineButton->setText(QCoreApplication::translate("MainWindow", "+", nullptr));
        delelineButton->setText(QCoreApplication::translate("MainWindow", "-", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "\344\277\235\345\255\230\345\210\260\357\274\232", nullptr));
        outputButton->setText(QCoreApplication::translate("MainWindow", "\346\265\217\350\247\210\342\200\246", nullptr));
        mergeStartBtn->setText(QCoreApplication::translate("MainWindow", "\345\220\257\345\212\250\346\267\267\346\265\201", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
