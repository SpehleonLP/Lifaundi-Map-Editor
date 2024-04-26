/********************************************************************************
** Form generated from reading UI file 'colorprogressindicator.ui'
**
** Created by: Qt User Interface Compiler version 6.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_COLORPROGRESSINDICATOR_H
#define UI_COLORPROGRESSINDICATOR_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ColorProgressIndicator
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QLabel *label_4;
    QLabel *label_5;
    QLabel *label_3;
    QLabel *label_2;
    QLineEdit *IslandsCompleted;
    QLineEdit *CurrentProgress;
    QLineEdit *TimesBacktracked;
    QLineEdit *CurrentColors;
    QLabel *Spinner;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *ColorProgressIndicator)
    {
        if (ColorProgressIndicator->objectName().isEmpty())
            ColorProgressIndicator->setObjectName("ColorProgressIndicator");
        ColorProgressIndicator->resize(369, 166);
        centralwidget = new QWidget(ColorProgressIndicator);
        centralwidget->setObjectName("centralwidget");
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName("gridLayout");
        label_4 = new QLabel(centralwidget);
        label_4->setObjectName("label_4");

        gridLayout->addWidget(label_4, 3, 0, 1, 1);

        label_5 = new QLabel(centralwidget);
        label_5->setObjectName("label_5");

        gridLayout->addWidget(label_5, 4, 0, 1, 1);

        label_3 = new QLabel(centralwidget);
        label_3->setObjectName("label_3");

        gridLayout->addWidget(label_3, 1, 0, 1, 1);

        label_2 = new QLabel(centralwidget);
        label_2->setObjectName("label_2");

        gridLayout->addWidget(label_2, 0, 0, 1, 1);

        IslandsCompleted = new QLineEdit(centralwidget);
        IslandsCompleted->setObjectName("IslandsCompleted");
        IslandsCompleted->setEnabled(false);

        gridLayout->addWidget(IslandsCompleted, 0, 1, 1, 1);

        CurrentProgress = new QLineEdit(centralwidget);
        CurrentProgress->setObjectName("CurrentProgress");
        CurrentProgress->setEnabled(false);

        gridLayout->addWidget(CurrentProgress, 1, 1, 1, 1);

        TimesBacktracked = new QLineEdit(centralwidget);
        TimesBacktracked->setObjectName("TimesBacktracked");
        TimesBacktracked->setEnabled(false);

        gridLayout->addWidget(TimesBacktracked, 3, 1, 1, 1);

        CurrentColors = new QLineEdit(centralwidget);
        CurrentColors->setObjectName("CurrentColors");
        CurrentColors->setEnabled(false);

        gridLayout->addWidget(CurrentColors, 4, 1, 1, 1);

        Spinner = new QLabel(centralwidget);
        Spinner->setObjectName("Spinner");

        gridLayout->addWidget(Spinner, 1, 3, 1, 2);

        ColorProgressIndicator->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(ColorProgressIndicator);
        statusbar->setObjectName("statusbar");
        ColorProgressIndicator->setStatusBar(statusbar);

        retranslateUi(ColorProgressIndicator);

        QMetaObject::connectSlotsByName(ColorProgressIndicator);
    } // setupUi

    void retranslateUi(QMainWindow *ColorProgressIndicator)
    {
        ColorProgressIndicator->setWindowTitle(QCoreApplication::translate("ColorProgressIndicator", "MainWindow", nullptr));
        label_4->setText(QCoreApplication::translate("ColorProgressIndicator", "Times Backtracked:", nullptr));
        label_5->setText(QCoreApplication::translate("ColorProgressIndicator", "Current Colors:", nullptr));
        label_3->setText(QCoreApplication::translate("ColorProgressIndicator", "Current Progress:", nullptr));
        label_2->setText(QCoreApplication::translate("ColorProgressIndicator", "Islands Completed:", nullptr));
        Spinner->setText(QCoreApplication::translate("ColorProgressIndicator", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ColorProgressIndicator: public Ui_ColorProgressIndicator {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_COLORPROGRESSINDICATOR_H
