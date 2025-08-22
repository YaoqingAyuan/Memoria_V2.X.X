/********************************************************************************
** Form generated from reading UI file 'setting_dialog.ui'
**
** Created by: Qt User Interface Compiler version 6.9.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SETTING_DIALOG_H
#define UI_SETTING_DIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Setting_Dialog
{
public:
    QTabWidget *tabWidget;
    QWidget *tabColumns;
    QLabel *label;
    QScrollArea *scrollArea;
    QWidget *columnsContainer;
    QWidget *tab_2;
    QPushButton *statesetting_Button;
    QLineEdit *del_state_line;
    QPushButton *statesetting_Button_2;
    QLineEdit *exp_stase_line;
    QPushButton *CancelButton;
    QPushButton *ApplyButton;
    QPushButton *OkButton;

    void setupUi(QDialog *Setting_Dialog)
    {
        if (Setting_Dialog->objectName().isEmpty())
            Setting_Dialog->setObjectName("Setting_Dialog");
        Setting_Dialog->resize(308, 348);
        tabWidget = new QTabWidget(Setting_Dialog);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setGeometry(QRect(7, 7, 291, 311));
        tabColumns = new QWidget();
        tabColumns->setObjectName("tabColumns");
        label = new QLabel(tabColumns);
        label->setObjectName("label");
        label->setGeometry(QRect(30, 6, 139, 20));
        QFont font;
        font.setFamilies({QString::fromUtf8("\345\276\256\350\275\257\351\233\205\351\273\221")});
        font.setPointSize(10);
        font.setUnderline(true);
        label->setFont(font);
        label->setTextFormat(Qt::TextFormat::AutoText);
        scrollArea = new QScrollArea(tabColumns);
        scrollArea->setObjectName("scrollArea");
        scrollArea->setGeometry(QRect(20, 30, 161, 191));
        scrollArea->setWidgetResizable(true);
        columnsContainer = new QWidget();
        columnsContainer->setObjectName("columnsContainer");
        columnsContainer->setGeometry(QRect(0, 0, 159, 189));
        scrollArea->setWidget(columnsContainer);
        tabWidget->addTab(tabColumns, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName("tab_2");
        statesetting_Button = new QPushButton(tab_2);
        statesetting_Button->setObjectName("statesetting_Button");
        statesetting_Button->setGeometry(QRect(30, 30, 80, 21));
        del_state_line = new QLineEdit(tab_2);
        del_state_line->setObjectName("del_state_line");
        del_state_line->setGeometry(QRect(110, 30, 151, 22));
        statesetting_Button_2 = new QPushButton(tab_2);
        statesetting_Button_2->setObjectName("statesetting_Button_2");
        statesetting_Button_2->setGeometry(QRect(30, 60, 80, 21));
        exp_stase_line = new QLineEdit(tab_2);
        exp_stase_line->setObjectName("exp_stase_line");
        exp_stase_line->setGeometry(QRect(110, 60, 151, 22));
        tabWidget->addTab(tab_2, QString());
        CancelButton = new QPushButton(Setting_Dialog);
        CancelButton->setObjectName("CancelButton");
        CancelButton->setGeometry(QRect(106, 321, 80, 21));
        ApplyButton = new QPushButton(Setting_Dialog);
        ApplyButton->setObjectName("ApplyButton");
        ApplyButton->setGeometry(QRect(191, 321, 80, 21));
        OkButton = new QPushButton(Setting_Dialog);
        OkButton->setObjectName("OkButton");
        OkButton->setGeometry(QRect(21, 321, 80, 21));

        retranslateUi(Setting_Dialog);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(Setting_Dialog);
    } // setupUi

    void retranslateUi(QDialog *Setting_Dialog)
    {
        Setting_Dialog->setWindowTitle(QCoreApplication::translate("Setting_Dialog", "Dialog", nullptr));
        label->setText(QCoreApplication::translate("Setting_Dialog", "\351\200\211\346\213\251\351\234\200\350\246\201\346\230\276\347\244\272\347\232\204\344\277\241\346\201\257\357\274\232", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabColumns), QCoreApplication::translate("Setting_Dialog", "\350\241\250\345\244\264\350\256\276\347\275\256", nullptr));
        statesetting_Button->setText(QCoreApplication::translate("Setting_Dialog", "\345\210\240\351\231\244\347\212\266\346\200\201\350\256\276\347\275\256", nullptr));
        statesetting_Button_2->setText(QCoreApplication::translate("Setting_Dialog", "\345\257\274\345\207\272\347\212\266\346\200\201\350\256\276\347\275\256", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("Setting_Dialog", "\346\234\252\345\221\275\345\220\215\350\256\276\347\275\256", nullptr));
        CancelButton->setText(QCoreApplication::translate("Setting_Dialog", "\345\217\226\346\266\210(C)", nullptr));
        ApplyButton->setText(QCoreApplication::translate("Setting_Dialog", "\345\272\224\347\224\250(A)", nullptr));
        OkButton->setText(QCoreApplication::translate("Setting_Dialog", "\347\241\256\345\256\232(O)", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Setting_Dialog: public Ui_Setting_Dialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SETTING_DIALOG_H
