/********************************************************************************
** Form generated from reading UI file 'exportoptions.ui'
**
** Created by: Qt User Interface Compiler version 6.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_EXPORTOPTIONS_H
#define UI_EXPORTOPTIONS_H

#include <QtCore/QVariant>
#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QSpinBox>

QT_BEGIN_NAMESPACE

class Ui_ExportOptions
{
public:
    QGridLayout *gridLayout;
    QSpinBox *posy;
    QLineEdit *background;
    QLabel *label_2;
    QSpinBox *height;
    QDialogButtonBox *buttonBox;
    QLabel *label;
    QSpinBox *posx;
    QLabel *label_3;
    QSpinBox *width;
    QLabel *label_5;
    QLabel *label_4;
    QLineEdit *mng;
    QLabel *label_6;

    void setupUi(QDialog *ExportOptions)
    {
        if (ExportOptions->objectName().isEmpty())
            ExportOptions->setObjectName("ExportOptions");
        ExportOptions->resize(329, 202);
        gridLayout = new QGridLayout(ExportOptions);
        gridLayout->setObjectName("gridLayout");
        posy = new QSpinBox(ExportOptions);
        posy->setObjectName("posy");
        posy->setMinimum(0);
        posy->setMaximum(100000);
        posy->setSingleStep(1000);
        posy->setValue(0);

        gridLayout->addWidget(posy, 3, 1, 1, 1);

        background = new QLineEdit(ExportOptions);
        background->setObjectName("background");

        gridLayout->addWidget(background, 0, 1, 1, 1);

        label_2 = new QLabel(ExportOptions);
        label_2->setObjectName("label_2");

        gridLayout->addWidget(label_2, 3, 0, 1, 1);

        height = new QSpinBox(ExportOptions);
        height->setObjectName("height");

        gridLayout->addWidget(height, 5, 1, 1, 1);

        buttonBox = new QDialogButtonBox(ExportOptions);
        buttonBox->setObjectName("buttonBox");
        buttonBox->setOrientation(Qt::Vertical);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 0, 2, 6, 1);

        label = new QLabel(ExportOptions);
        label->setObjectName("label");

        gridLayout->addWidget(label, 2, 0, 1, 1);

        posx = new QSpinBox(ExportOptions);
        posx->setObjectName("posx");
        posx->setMaximum(100000);
        posx->setSingleStep(1000);

        gridLayout->addWidget(posx, 2, 1, 1, 1);

        label_3 = new QLabel(ExportOptions);
        label_3->setObjectName("label_3");

        gridLayout->addWidget(label_3, 4, 0, 1, 1);

        width = new QSpinBox(ExportOptions);
        width->setObjectName("width");

        gridLayout->addWidget(width, 4, 1, 1, 1);

        label_5 = new QLabel(ExportOptions);
        label_5->setObjectName("label_5");

        gridLayout->addWidget(label_5, 0, 0, 1, 1);

        label_4 = new QLabel(ExportOptions);
        label_4->setObjectName("label_4");

        gridLayout->addWidget(label_4, 5, 0, 1, 1);

        mng = new QLineEdit(ExportOptions);
        mng->setObjectName("mng");

        gridLayout->addWidget(mng, 1, 1, 1, 1);

        label_6 = new QLabel(ExportOptions);
        label_6->setObjectName("label_6");

        gridLayout->addWidget(label_6, 1, 0, 1, 1);


        retranslateUi(ExportOptions);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, ExportOptions, qOverload<>(&QDialog::accept));
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, ExportOptions, qOverload<>(&QDialog::reject));

        QMetaObject::connectSlotsByName(ExportOptions);
    } // setupUi

    void retranslateUi(QDialog *ExportOptions)
    {
        ExportOptions->setWindowTitle(QCoreApplication::translate("ExportOptions", "Dialog", nullptr));
        label_2->setText(QCoreApplication::translate("ExportOptions", "Position Y", nullptr));
        label->setText(QCoreApplication::translate("ExportOptions", "Position X", nullptr));
        label_3->setText(QCoreApplication::translate("ExportOptions", "Width", nullptr));
        label_5->setText(QCoreApplication::translate("ExportOptions", "Background", nullptr));
        label_4->setText(QCoreApplication::translate("ExportOptions", "Height", nullptr));
        label_6->setText(QCoreApplication::translate("ExportOptions", "Music", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ExportOptions: public Ui_ExportOptions {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_EXPORTOPTIONS_H
