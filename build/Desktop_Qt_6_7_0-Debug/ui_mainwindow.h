/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.7.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDial>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <src/glviewwidget.h>
#include "src/histogramwidget.h"
#include "src/widget_rangeslider.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *fileNew;
    QAction *fileOpen;
    QAction *fileSaveAs;
    QAction *fileSave;
    QAction *fileClose;
    QAction *appExit;
    QAction *editUndo;
    QAction *editRedo;
    QAction *selectAll;
    QAction *zoomIn;
    QAction *zoomOut;
    QAction *zoomActual;
    QAction *selectNone;
    QAction *editCut;
    QAction *editCopy;
    QAction *editPaste;
    QAction *editDelete;
    QAction *viewChecker;
    QAction *viewRooms;
    QAction *viewLinks;
    QAction *aboutQt;
    QAction *aboutAbout;
    QAction *aboutHelp;
    QAction *linkAdd;
    QAction *linkRemove;
    QAction *viewHalls;
    QAction *toolNew;
    QAction *toolExtrude;
    QAction *toolSlice;
    QAction *musicMute;
    QAction *fileImportWalls;
    QAction *fileLoadBackground;
    QAction *musicLoad;
    QAction *toolTranslate;
    QAction *toolRotate;
    QAction *toolScale;
    QAction *fileSaveRooms;
    QAction *toolReseat;
    QAction *fileCAOS;
    QAction *viewGravity;
    QAction *toolDuplicate;
    QAction *toolFace;
    QAction *toolRealign;
    QAction *toolPropagate;
    QAction *selectInvert;
    QAction *selectConnected;
    QAction *selectOverlap;
    QAction *selectRoomType;
    QAction *selectRoomMusic;
    QAction *debugTreeSymmetry;
    QAction *debugDoorSymmetry;
    QAction *viewRoomId;
    QAction *debugSelectRoom;
    QAction *debugDumpPermTable;
    QAction *toolAutoReseat;
    QAction *layerBaseColor;
    QAction *layerNormals;
    QAction *layerOcclusion;
    QAction *layerRoughness;
    QAction *layerDepth;
    QWidget *centralWidget;
    QHBoxLayout *horizontalLayout_3;
    QWidget *widget;
    QVBoxLayout *verticalLayout_4;
    QGroupBox *groupBox_5;
    QFormLayout *formLayout;
    QLabel *Permeability;
    QSpinBox *permeability;
    QGroupBox *groupBox_2;
    QVBoxLayout *verticalLayout_3;
    QFormLayout *formLayout_5;
    QLabel *label_20;
    QComboBox *room_music;
    QLabel *label_2;
    QComboBox *room_type;
    QGroupBox *groupBox;
    QFormLayout *formLayout_3;
    QLabel *label_6;
    QDoubleSpinBox *gravityStrength;
    QVBoxLayout *verticalLayout;
    QLabel *label_5;
    QSpacerItem *verticalSpacer_3;
    QHBoxLayout *horizontalLayout;
    QDial *gravityDir;
    QSpacerItem *horizontalSpacer;
    QGroupBox *groupBox_6;
    QGridLayout *gridLayout;
    QLabel *label_9;
    QSlider *ambientShade;
    QLabel *label_7;
    QDoubleSpinBox *shadeStrength;
    QVBoxLayout *verticalLayout_2;
    QLabel *label_8;
    QSpacerItem *verticalSpacer_4;
    QHBoxLayout *horizontalLayout_2;
    QDial *shadeDir;
    QSpacerItem *horizontalSpacer_2;
    QGroupBox *groupBox_3;
    QFormLayout *formLayout_4;
    QLabel *label_13;
    QLabel *label_10;
    QLabel *label_11;
    QLabel *label_12;
    QSlider *audio0;
    QSlider *audio1;
    QSlider *audio2;
    QSlider *audio3;
    QGroupBox *groupBox_7;
    QVBoxLayout *verticalLayout_6;
    HistogramWidget *roomDepthHistogram;
    RangeSlider *roomDepthSlider;
    QLabel *roomDepthLabel;
    QGroupBox *groupBox_4;
    QVBoxLayout *verticalLayout_5;
    HistogramWidget *backgroundHistogram;
    RangeSlider *backgroundDepthSlider;
    QLabel *backgroundDepthLabel;
    QSpacerItem *verticalSpacer;
    QGridLayout *gridLayout_4;
    QScrollBar *horizontalScrollBar;
    GLViewWidget *viewWidget;
    QScrollBar *verticalScrollBar;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuEdit;
    QMenu *menuZoom;
    QMenu *menuSelect_2;
    QMenu *menuView;
    QMenu *menuAbout;
    QMenu *menuLinks;
    QMenu *menuTools;
    QMenu *menuDebug;
    QMenu *menuBackground_Layer;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(962, 996);
        QSizePolicy sizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        fileNew = new QAction(MainWindow);
        fileNew->setObjectName("fileNew");
        QIcon icon(QIcon::fromTheme(QString::fromUtf8("document-new")));
        fileNew->setIcon(icon);
        fileOpen = new QAction(MainWindow);
        fileOpen->setObjectName("fileOpen");
        QIcon icon1(QIcon::fromTheme(QString::fromUtf8("document-open")));
        fileOpen->setIcon(icon1);
        fileSaveAs = new QAction(MainWindow);
        fileSaveAs->setObjectName("fileSaveAs");
        QIcon icon2(QIcon::fromTheme(QString::fromUtf8("document-save-as")));
        fileSaveAs->setIcon(icon2);
        fileSave = new QAction(MainWindow);
        fileSave->setObjectName("fileSave");
        QIcon icon3(QIcon::fromTheme(QString::fromUtf8("document-save")));
        fileSave->setIcon(icon3);
        fileClose = new QAction(MainWindow);
        fileClose->setObjectName("fileClose");
        QIcon icon4(QIcon::fromTheme(QString::fromUtf8("document-close")));
        fileClose->setIcon(icon4);
        appExit = new QAction(MainWindow);
        appExit->setObjectName("appExit");
        QIcon icon5(QIcon::fromTheme(QString::fromUtf8("application-exit")));
        appExit->setIcon(icon5);
        editUndo = new QAction(MainWindow);
        editUndo->setObjectName("editUndo");
        editUndo->setEnabled(false);
        QIcon icon6(QIcon::fromTheme(QString::fromUtf8("edit-undo")));
        editUndo->setIcon(icon6);
        editRedo = new QAction(MainWindow);
        editRedo->setObjectName("editRedo");
        editRedo->setEnabled(false);
        QIcon icon7(QIcon::fromTheme(QString::fromUtf8("edit-redo")));
        editRedo->setIcon(icon7);
        selectAll = new QAction(MainWindow);
        selectAll->setObjectName("selectAll");
        selectAll->setEnabled(true);
        QIcon icon8(QIcon::fromTheme(QString::fromUtf8("select")));
        selectAll->setIcon(icon8);
        zoomIn = new QAction(MainWindow);
        zoomIn->setObjectName("zoomIn");
        QIcon icon9(QIcon::fromTheme(QString::fromUtf8("zoom-in")));
        zoomIn->setIcon(icon9);
        zoomOut = new QAction(MainWindow);
        zoomOut->setObjectName("zoomOut");
        QIcon icon10(QIcon::fromTheme(QString::fromUtf8("zoom-out")));
        zoomOut->setIcon(icon10);
        zoomActual = new QAction(MainWindow);
        zoomActual->setObjectName("zoomActual");
        QIcon icon11(QIcon::fromTheme(QString::fromUtf8("zoom")));
        zoomActual->setIcon(icon11);
        selectNone = new QAction(MainWindow);
        selectNone->setObjectName("selectNone");
        selectNone->setEnabled(true);
        editCut = new QAction(MainWindow);
        editCut->setObjectName("editCut");
        editCut->setEnabled(true);
        QIcon icon12(QIcon::fromTheme(QString::fromUtf8("edit-cut")));
        editCut->setIcon(icon12);
        editCopy = new QAction(MainWindow);
        editCopy->setObjectName("editCopy");
        editCopy->setEnabled(true);
        QIcon icon13(QIcon::fromTheme(QString::fromUtf8("edit-copy")));
        editCopy->setIcon(icon13);
        editPaste = new QAction(MainWindow);
        editPaste->setObjectName("editPaste");
        editPaste->setEnabled(true);
        QIcon icon14(QIcon::fromTheme(QString::fromUtf8("edit-paste")));
        editPaste->setIcon(icon14);
        editDelete = new QAction(MainWindow);
        editDelete->setObjectName("editDelete");
        editDelete->setEnabled(true);
        QIcon icon15(QIcon::fromTheme(QString::fromUtf8("edit-delete")));
        editDelete->setIcon(icon15);
        viewChecker = new QAction(MainWindow);
        viewChecker->setObjectName("viewChecker");
        viewChecker->setCheckable(true);
        viewChecker->setChecked(true);
        viewRooms = new QAction(MainWindow);
        viewRooms->setObjectName("viewRooms");
        viewRooms->setCheckable(true);
        viewRooms->setChecked(true);
        viewLinks = new QAction(MainWindow);
        viewLinks->setObjectName("viewLinks");
        viewLinks->setCheckable(true);
        aboutQt = new QAction(MainWindow);
        aboutQt->setObjectName("aboutQt");
        aboutAbout = new QAction(MainWindow);
        aboutAbout->setObjectName("aboutAbout");
        aboutHelp = new QAction(MainWindow);
        aboutHelp->setObjectName("aboutHelp");
        linkAdd = new QAction(MainWindow);
        linkAdd->setObjectName("linkAdd");
        linkAdd->setEnabled(false);
        linkRemove = new QAction(MainWindow);
        linkRemove->setObjectName("linkRemove");
        linkRemove->setEnabled(false);
        viewHalls = new QAction(MainWindow);
        viewHalls->setObjectName("viewHalls");
        viewHalls->setCheckable(true);
        toolNew = new QAction(MainWindow);
        toolNew->setObjectName("toolNew");
        toolExtrude = new QAction(MainWindow);
        toolExtrude->setObjectName("toolExtrude");
        toolExtrude->setEnabled(false);
        toolSlice = new QAction(MainWindow);
        toolSlice->setObjectName("toolSlice");
        toolSlice->setEnabled(true);
        musicMute = new QAction(MainWindow);
        musicMute->setObjectName("musicMute");
        musicMute->setCheckable(true);
        musicMute->setChecked(true);
        fileImportWalls = new QAction(MainWindow);
        fileImportWalls->setObjectName("fileImportWalls");
        fileLoadBackground = new QAction(MainWindow);
        fileLoadBackground->setObjectName("fileLoadBackground");
        musicLoad = new QAction(MainWindow);
        musicLoad->setObjectName("musicLoad");
        toolTranslate = new QAction(MainWindow);
        toolTranslate->setObjectName("toolTranslate");
        toolRotate = new QAction(MainWindow);
        toolRotate->setObjectName("toolRotate");
        toolScale = new QAction(MainWindow);
        toolScale->setObjectName("toolScale");
        fileSaveRooms = new QAction(MainWindow);
        fileSaveRooms->setObjectName("fileSaveRooms");
        toolReseat = new QAction(MainWindow);
        toolReseat->setObjectName("toolReseat");
        fileCAOS = new QAction(MainWindow);
        fileCAOS->setObjectName("fileCAOS");
        fileCAOS->setEnabled(true);
        viewGravity = new QAction(MainWindow);
        viewGravity->setObjectName("viewGravity");
        viewGravity->setCheckable(true);
        toolDuplicate = new QAction(MainWindow);
        toolDuplicate->setObjectName("toolDuplicate");
        toolFace = new QAction(MainWindow);
        toolFace->setObjectName("toolFace");
        toolRealign = new QAction(MainWindow);
        toolRealign->setObjectName("toolRealign");
        toolPropagate = new QAction(MainWindow);
        toolPropagate->setObjectName("toolPropagate");
        selectInvert = new QAction(MainWindow);
        selectInvert->setObjectName("selectInvert");
        selectConnected = new QAction(MainWindow);
        selectConnected->setObjectName("selectConnected");
        selectOverlap = new QAction(MainWindow);
        selectOverlap->setObjectName("selectOverlap");
        selectRoomType = new QAction(MainWindow);
        selectRoomType->setObjectName("selectRoomType");
        selectRoomMusic = new QAction(MainWindow);
        selectRoomMusic->setObjectName("selectRoomMusic");
        debugTreeSymmetry = new QAction(MainWindow);
        debugTreeSymmetry->setObjectName("debugTreeSymmetry");
        debugDoorSymmetry = new QAction(MainWindow);
        debugDoorSymmetry->setObjectName("debugDoorSymmetry");
        viewRoomId = new QAction(MainWindow);
        viewRoomId->setObjectName("viewRoomId");
        viewRoomId->setCheckable(true);
        debugSelectRoom = new QAction(MainWindow);
        debugSelectRoom->setObjectName("debugSelectRoom");
        debugDumpPermTable = new QAction(MainWindow);
        debugDumpPermTable->setObjectName("debugDumpPermTable");
        toolAutoReseat = new QAction(MainWindow);
        toolAutoReseat->setObjectName("toolAutoReseat");
        layerBaseColor = new QAction(MainWindow);
        layerBaseColor->setObjectName("layerBaseColor");
        layerBaseColor->setCheckable(true);
        layerBaseColor->setChecked(true);
        layerBaseColor->setEnabled(true);
        layerNormals = new QAction(MainWindow);
        layerNormals->setObjectName("layerNormals");
        layerNormals->setCheckable(true);
        layerNormals->setChecked(false);
        layerNormals->setEnabled(true);
        layerOcclusion = new QAction(MainWindow);
        layerOcclusion->setObjectName("layerOcclusion");
        layerOcclusion->setCheckable(true);
        layerOcclusion->setChecked(false);
        layerOcclusion->setEnabled(true);
        layerRoughness = new QAction(MainWindow);
        layerRoughness->setObjectName("layerRoughness");
        layerRoughness->setCheckable(true);
        layerRoughness->setChecked(false);
        layerRoughness->setEnabled(true);
        layerDepth = new QAction(MainWindow);
        layerDepth->setObjectName("layerDepth");
        layerDepth->setCheckable(true);
        layerDepth->setChecked(false);
        layerDepth->setEnabled(true);
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName("centralWidget");
        horizontalLayout_3 = new QHBoxLayout(centralWidget);
        horizontalLayout_3->setSpacing(0);
        horizontalLayout_3->setContentsMargins(11, 11, 11, 11);
        horizontalLayout_3->setObjectName("horizontalLayout_3");
        horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
        widget = new QWidget(centralWidget);
        widget->setObjectName("widget");
        QSizePolicy sizePolicy1(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(widget->sizePolicy().hasHeightForWidth());
        widget->setSizePolicy(sizePolicy1);
        verticalLayout_4 = new QVBoxLayout(widget);
        verticalLayout_4->setSpacing(6);
        verticalLayout_4->setContentsMargins(11, 11, 11, 11);
        verticalLayout_4->setObjectName("verticalLayout_4");
        groupBox_5 = new QGroupBox(widget);
        groupBox_5->setObjectName("groupBox_5");
        formLayout = new QFormLayout(groupBox_5);
        formLayout->setSpacing(6);
        formLayout->setContentsMargins(11, 11, 11, 11);
        formLayout->setObjectName("formLayout");
        Permeability = new QLabel(groupBox_5);
        Permeability->setObjectName("Permeability");

        formLayout->setWidget(0, QFormLayout::LabelRole, Permeability);

        permeability = new QSpinBox(groupBox_5);
        permeability->setObjectName("permeability");
        permeability->setMinimum(-1);
        permeability->setMaximum(100);

        formLayout->setWidget(0, QFormLayout::FieldRole, permeability);


        verticalLayout_4->addWidget(groupBox_5);

        groupBox_2 = new QGroupBox(widget);
        groupBox_2->setObjectName("groupBox_2");
        verticalLayout_3 = new QVBoxLayout(groupBox_2);
        verticalLayout_3->setSpacing(6);
        verticalLayout_3->setContentsMargins(11, 11, 11, 11);
        verticalLayout_3->setObjectName("verticalLayout_3");
        verticalLayout_3->setContentsMargins(4, -1, 4, -1);
        formLayout_5 = new QFormLayout();
        formLayout_5->setSpacing(6);
        formLayout_5->setObjectName("formLayout_5");
        label_20 = new QLabel(groupBox_2);
        label_20->setObjectName("label_20");

        formLayout_5->setWidget(0, QFormLayout::LabelRole, label_20);

        room_music = new QComboBox(groupBox_2);
        room_music->setObjectName("room_music");
        QSizePolicy sizePolicy2(QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(room_music->sizePolicy().hasHeightForWidth());
        room_music->setSizePolicy(sizePolicy2);
        room_music->setEditable(true);

        formLayout_5->setWidget(0, QFormLayout::FieldRole, room_music);

        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName("label_2");

        formLayout_5->setWidget(1, QFormLayout::LabelRole, label_2);

        room_type = new QComboBox(groupBox_2);
        room_type->setObjectName("room_type");
        sizePolicy2.setHeightForWidth(room_type->sizePolicy().hasHeightForWidth());
        room_type->setSizePolicy(sizePolicy2);
        room_type->setMaxCount(16);

        formLayout_5->setWidget(1, QFormLayout::FieldRole, room_type);


        verticalLayout_3->addLayout(formLayout_5);

        groupBox = new QGroupBox(groupBox_2);
        groupBox->setObjectName("groupBox");
        sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy);
        formLayout_3 = new QFormLayout(groupBox);
        formLayout_3->setSpacing(6);
        formLayout_3->setContentsMargins(11, 11, 11, 11);
        formLayout_3->setObjectName("formLayout_3");
        label_6 = new QLabel(groupBox);
        label_6->setObjectName("label_6");

        formLayout_3->setWidget(0, QFormLayout::LabelRole, label_6);

        gravityStrength = new QDoubleSpinBox(groupBox);
        gravityStrength->setObjectName("gravityStrength");
        gravityStrength->setEnabled(false);
        sizePolicy2.setHeightForWidth(gravityStrength->sizePolicy().hasHeightForWidth());
        gravityStrength->setSizePolicy(sizePolicy2);
        gravityStrength->setDecimals(4);
        gravityStrength->setMinimum(-20.000000000000000);
        gravityStrength->setMaximum(20.000000000000000);

        formLayout_3->setWidget(0, QFormLayout::FieldRole, gravityStrength);

        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName("verticalLayout");
        label_5 = new QLabel(groupBox);
        label_5->setObjectName("label_5");

        verticalLayout->addWidget(label_5);

        verticalSpacer_3 = new QSpacerItem(20, 1, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);

        verticalLayout->addItem(verticalSpacer_3);


        formLayout_3->setLayout(1, QFormLayout::LabelRole, verticalLayout);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(0);
        horizontalLayout->setObjectName("horizontalLayout");
        gravityDir = new QDial(groupBox);
        gravityDir->setObjectName("gravityDir");
        gravityDir->setMaximumSize(QSize(16777215, 48));
        gravityDir->setMinimum(0);
        gravityDir->setMaximum(3600);
        gravityDir->setSingleStep(150);
        gravityDir->setPageStep(600);
        gravityDir->setValue(1800);
        gravityDir->setInvertedAppearance(false);
        gravityDir->setWrapping(true);
        gravityDir->setNotchTarget(1.000000000000000);
        gravityDir->setNotchesVisible(true);

        horizontalLayout->addWidget(gravityDir);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        formLayout_3->setLayout(1, QFormLayout::FieldRole, horizontalLayout);


        verticalLayout_3->addWidget(groupBox);

        groupBox_6 = new QGroupBox(groupBox_2);
        groupBox_6->setObjectName("groupBox_6");
        sizePolicy.setHeightForWidth(groupBox_6->sizePolicy().hasHeightForWidth());
        groupBox_6->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(groupBox_6);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName("gridLayout");
        label_9 = new QLabel(groupBox_6);
        label_9->setObjectName("label_9");

        gridLayout->addWidget(label_9, 0, 0, 1, 1);

        ambientShade = new QSlider(groupBox_6);
        ambientShade->setObjectName("ambientShade");
        ambientShade->setMaximum(255);
        ambientShade->setSingleStep(4);
        ambientShade->setPageStep(32);
        ambientShade->setOrientation(Qt::Horizontal);

        gridLayout->addWidget(ambientShade, 0, 1, 1, 1);

        label_7 = new QLabel(groupBox_6);
        label_7->setObjectName("label_7");

        gridLayout->addWidget(label_7, 1, 0, 1, 1);

        shadeStrength = new QDoubleSpinBox(groupBox_6);
        shadeStrength->setObjectName("shadeStrength");
        shadeStrength->setEnabled(false);
        sizePolicy2.setHeightForWidth(shadeStrength->sizePolicy().hasHeightForWidth());
        shadeStrength->setSizePolicy(sizePolicy2);
        shadeStrength->setDecimals(4);
        shadeStrength->setMinimum(-20.000000000000000);
        shadeStrength->setMaximum(20.000000000000000);

        gridLayout->addWidget(shadeStrength, 1, 1, 1, 1);

        verticalLayout_2 = new QVBoxLayout();
        verticalLayout_2->setSpacing(0);
        verticalLayout_2->setObjectName("verticalLayout_2");
        label_8 = new QLabel(groupBox_6);
        label_8->setObjectName("label_8");

        verticalLayout_2->addWidget(label_8);

        verticalSpacer_4 = new QSpacerItem(20, 1, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);

        verticalLayout_2->addItem(verticalSpacer_4);


        gridLayout->addLayout(verticalLayout_2, 2, 0, 1, 1);

        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setSpacing(0);
        horizontalLayout_2->setObjectName("horizontalLayout_2");
        shadeDir = new QDial(groupBox_6);
        shadeDir->setObjectName("shadeDir");
        shadeDir->setMaximumSize(QSize(16777215, 48));
        shadeDir->setMinimum(0);
        shadeDir->setMaximum(3600);
        shadeDir->setSingleStep(150);
        shadeDir->setPageStep(600);
        shadeDir->setValue(1800);
        shadeDir->setInvertedAppearance(false);
        shadeDir->setWrapping(true);
        shadeDir->setNotchTarget(1.000000000000000);
        shadeDir->setNotchesVisible(true);

        horizontalLayout_2->addWidget(shadeDir);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);


        gridLayout->addLayout(horizontalLayout_2, 2, 1, 1, 1);


        verticalLayout_3->addWidget(groupBox_6);

        groupBox_3 = new QGroupBox(groupBox_2);
        groupBox_3->setObjectName("groupBox_3");
        sizePolicy1.setHeightForWidth(groupBox_3->sizePolicy().hasHeightForWidth());
        groupBox_3->setSizePolicy(sizePolicy1);
        formLayout_4 = new QFormLayout(groupBox_3);
        formLayout_4->setSpacing(6);
        formLayout_4->setContentsMargins(11, 11, 11, 11);
        formLayout_4->setObjectName("formLayout_4");
        label_13 = new QLabel(groupBox_3);
        label_13->setObjectName("label_13");

        formLayout_4->setWidget(0, QFormLayout::LabelRole, label_13);

        label_10 = new QLabel(groupBox_3);
        label_10->setObjectName("label_10");

        formLayout_4->setWidget(1, QFormLayout::LabelRole, label_10);

        label_11 = new QLabel(groupBox_3);
        label_11->setObjectName("label_11");

        formLayout_4->setWidget(2, QFormLayout::LabelRole, label_11);

        label_12 = new QLabel(groupBox_3);
        label_12->setObjectName("label_12");

        formLayout_4->setWidget(3, QFormLayout::LabelRole, label_12);

        audio0 = new QSlider(groupBox_3);
        audio0->setObjectName("audio0");
        audio0->setEnabled(false);
        sizePolicy2.setHeightForWidth(audio0->sizePolicy().hasHeightForWidth());
        audio0->setSizePolicy(sizePolicy2);
        audio0->setMaximum(255);
        audio0->setSingleStep(4);
        audio0->setPageStep(32);
        audio0->setOrientation(Qt::Horizontal);

        formLayout_4->setWidget(0, QFormLayout::FieldRole, audio0);

        audio1 = new QSlider(groupBox_3);
        audio1->setObjectName("audio1");
        audio1->setEnabled(false);
        sizePolicy2.setHeightForWidth(audio1->sizePolicy().hasHeightForWidth());
        audio1->setSizePolicy(sizePolicy2);
        audio1->setMaximum(255);
        audio1->setSingleStep(4);
        audio1->setPageStep(32);
        audio1->setOrientation(Qt::Horizontal);

        formLayout_4->setWidget(1, QFormLayout::FieldRole, audio1);

        audio2 = new QSlider(groupBox_3);
        audio2->setObjectName("audio2");
        audio2->setEnabled(false);
        sizePolicy2.setHeightForWidth(audio2->sizePolicy().hasHeightForWidth());
        audio2->setSizePolicy(sizePolicy2);
        audio2->setMaximum(255);
        audio2->setSingleStep(4);
        audio2->setPageStep(32);
        audio2->setOrientation(Qt::Horizontal);

        formLayout_4->setWidget(2, QFormLayout::FieldRole, audio2);

        audio3 = new QSlider(groupBox_3);
        audio3->setObjectName("audio3");
        audio3->setEnabled(false);
        sizePolicy2.setHeightForWidth(audio3->sizePolicy().hasHeightForWidth());
        audio3->setSizePolicy(sizePolicy2);
        audio3->setMaximum(255);
        audio3->setSingleStep(4);
        audio3->setPageStep(32);
        audio3->setOrientation(Qt::Horizontal);

        formLayout_4->setWidget(3, QFormLayout::FieldRole, audio3);


        verticalLayout_3->addWidget(groupBox_3);

        groupBox_7 = new QGroupBox(groupBox_2);
        groupBox_7->setObjectName("groupBox_7");
        verticalLayout_6 = new QVBoxLayout(groupBox_7);
        verticalLayout_6->setSpacing(6);
        verticalLayout_6->setContentsMargins(11, 11, 11, 11);
        verticalLayout_6->setObjectName("verticalLayout_6");
		verticalLayout_6->setContentsMargins(5, -1, 5, -1);

        roomDepthSlider = new RangeSlider(groupBox_7);
        roomDepthSlider->setObjectName("roomDepthSlider");
        roomDepthSlider->setMaximum(256);
        roomDepthSlider->setSingleStep(4);
        roomDepthSlider->setPageStep(16);
        roomDepthSlider->setOrientation(Qt::Horizontal);
        roomDepthSlider->setTickPosition(QSlider::TicksBelow);
        roomDepthSlider->setTickInterval(16);

        verticalLayout_6->addWidget(roomDepthSlider);

        roomDepthLabel = new QLabel(groupBox_7);
        roomDepthLabel->setObjectName("roomDepthLabel");

        verticalLayout_6->addWidget(roomDepthLabel);


        verticalLayout_3->addWidget(groupBox_7);


        verticalLayout_4->addWidget(groupBox_2);

        groupBox_4 = new QGroupBox(widget);
        groupBox_4->setObjectName("groupBox_4");
        verticalLayout_5 = new QVBoxLayout(groupBox_4);
        verticalLayout_5->setSpacing(6);
        verticalLayout_5->setContentsMargins(11, 11, 11, 11);
		verticalLayout_5->setObjectName("verticalLayout_5");

        backgroundDepthSlider = new RangeSlider(groupBox_4);
        backgroundDepthSlider->setObjectName("backgroundDepthSlider");
        backgroundDepthSlider->setMaximum(256);
        backgroundDepthSlider->setSingleStep(4);
        backgroundDepthSlider->setPageStep(16);
        backgroundDepthSlider->setOrientation(Qt::Horizontal);
        backgroundDepthSlider->setTickPosition(QSlider::TicksBelow);
        backgroundDepthSlider->setTickInterval(16);

        verticalLayout_5->addWidget(backgroundDepthSlider);

        backgroundDepthLabel = new QLabel(groupBox_4);
        backgroundDepthLabel->setObjectName("backgroundDepthLabel");

        verticalLayout_5->addWidget(backgroundDepthLabel);


        verticalLayout_4->addWidget(groupBox_4);

        verticalSpacer = new QSpacerItem(20, 24, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);

        verticalLayout_4->addItem(verticalSpacer);


        horizontalLayout_3->addWidget(widget);

        gridLayout_4 = new QGridLayout();
        gridLayout_4->setSpacing(0);
        gridLayout_4->setObjectName("gridLayout_4");
        horizontalScrollBar = new QScrollBar(centralWidget);
        horizontalScrollBar->setObjectName("horizontalScrollBar");
        horizontalScrollBar->setMaximum(65535);
        horizontalScrollBar->setSingleStep(1024);
        horizontalScrollBar->setPageStep(4096);
        horizontalScrollBar->setValue(32767);
        horizontalScrollBar->setOrientation(Qt::Horizontal);

        gridLayout_4->addWidget(horizontalScrollBar, 1, 0, 1, 1);

        viewWidget = new GLViewWidget(centralWidget);
        viewWidget->setObjectName("viewWidget");
        QSizePolicy sizePolicy3(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(viewWidget->sizePolicy().hasHeightForWidth());
        viewWidget->setSizePolicy(sizePolicy3);
        viewWidget->setMinimumSize(QSize(640, 480));

        gridLayout_4->addWidget(viewWidget, 0, 0, 1, 1);

        verticalScrollBar = new QScrollBar(centralWidget);
        verticalScrollBar->setObjectName("verticalScrollBar");
        verticalScrollBar->setMaximum(65535);
        verticalScrollBar->setSingleStep(1024);
        verticalScrollBar->setPageStep(4096);
        verticalScrollBar->setValue(32767);
        verticalScrollBar->setSliderPosition(32767);
        verticalScrollBar->setOrientation(Qt::Vertical);

        gridLayout_4->addWidget(verticalScrollBar, 0, 1, 1, 1);


        horizontalLayout_3->addLayout(gridLayout_4);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName("menuBar");
        menuBar->setGeometry(QRect(0, 0, 962, 20));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName("menuFile");
        menuEdit = new QMenu(menuBar);
        menuEdit->setObjectName("menuEdit");
        menuZoom = new QMenu(menuEdit);
        menuZoom->setObjectName("menuZoom");
        menuSelect_2 = new QMenu(menuEdit);
        menuSelect_2->setObjectName("menuSelect_2");
        menuView = new QMenu(menuBar);
        menuView->setObjectName("menuView");
        menuAbout = new QMenu(menuBar);
        menuAbout->setObjectName("menuAbout");
        menuLinks = new QMenu(menuBar);
        menuLinks->setObjectName("menuLinks");
        menuTools = new QMenu(menuBar);
        menuTools->setObjectName("menuTools");
        menuDebug = new QMenu(menuBar);
        menuDebug->setObjectName("menuDebug");
        menuBackground_Layer = new QMenu(menuBar);
        menuBackground_Layer->setObjectName("menuBackground_Layer");
        MainWindow->setMenuBar(menuBar);
        statusBar = new QStatusBar(MainWindow);
        statusBar->setObjectName("statusBar");
        statusBar->setSizeGripEnabled(false);
        MainWindow->setStatusBar(statusBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuEdit->menuAction());
        menuBar->addAction(menuTools->menuAction());
        menuBar->addAction(menuLinks->menuAction());
        menuBar->addAction(menuView->menuAction());
        menuBar->addAction(menuBackground_Layer->menuAction());
        menuBar->addAction(menuDebug->menuAction());
        menuBar->addAction(menuAbout->menuAction());
        menuFile->addAction(fileNew);
        menuFile->addAction(fileOpen);
        menuFile->addSeparator();
        menuFile->addAction(fileSave);
        menuFile->addAction(fileSaveAs);
        menuFile->addSeparator();
        menuFile->addAction(fileLoadBackground);
        menuFile->addAction(fileImportWalls);
        menuFile->addAction(fileCAOS);
        menuFile->addSeparator();
        menuFile->addAction(fileClose);
        menuFile->addAction(appExit);
        menuEdit->addAction(editUndo);
        menuEdit->addAction(editRedo);
        menuEdit->addSeparator();
        menuEdit->addAction(editCut);
        menuEdit->addAction(editCopy);
        menuEdit->addAction(editPaste);
        menuEdit->addAction(editDelete);
        menuEdit->addSeparator();
        menuEdit->addAction(menuZoom->menuAction());
        menuEdit->addAction(menuSelect_2->menuAction());
        menuZoom->addAction(zoomIn);
        menuZoom->addAction(zoomOut);
        menuZoom->addAction(zoomActual);
        menuSelect_2->addAction(selectAll);
        menuSelect_2->addAction(selectNone);
        menuSelect_2->addAction(selectInvert);
        menuSelect_2->addAction(selectConnected);
        menuSelect_2->addAction(selectOverlap);
        menuSelect_2->addAction(selectRoomType);
        menuSelect_2->addAction(selectRoomMusic);
        menuView->addAction(viewChecker);
        menuView->addAction(viewRooms);
        menuView->addAction(viewHalls);
        menuView->addAction(viewLinks);
        menuView->addSeparator();
        menuView->addAction(viewGravity);
        menuAbout->addAction(aboutHelp);
        menuAbout->addAction(aboutAbout);
        menuAbout->addSeparator();
        menuAbout->addAction(aboutQt);
        menuLinks->addAction(linkAdd);
        menuLinks->addAction(linkRemove);
        menuTools->addAction(toolNew);
        menuTools->addAction(toolFace);
        menuTools->addAction(toolDuplicate);
        menuTools->addSeparator();
        menuTools->addAction(toolReseat);
        menuTools->addSeparator();
        menuTools->addAction(toolTranslate);
        menuTools->addAction(toolRotate);
        menuTools->addAction(toolScale);
        menuTools->addAction(toolExtrude);
        menuTools->addAction(toolSlice);
        menuTools->addSeparator();
        menuTools->addAction(toolRealign);
        menuTools->addAction(toolPropagate);
        menuTools->addAction(toolAutoReseat);
        menuDebug->addAction(debugTreeSymmetry);
        menuDebug->addAction(debugDoorSymmetry);
        menuDebug->addSeparator();
        menuDebug->addAction(viewRoomId);
        menuDebug->addAction(debugSelectRoom);
        menuDebug->addAction(debugDumpPermTable);
        menuBackground_Layer->addAction(layerBaseColor);
        menuBackground_Layer->addAction(layerNormals);
        menuBackground_Layer->addAction(layerOcclusion);
        menuBackground_Layer->addAction(layerRoughness);
        menuBackground_Layer->addAction(layerDepth);

		roomDepthHistogram = new HistogramWidget(groupBox_7);
		roomDepthHistogram->setObjectName("roomDepthHistogram");
		roomDepthHistogram->setMinimumSize(QSize(0, 64));

		verticalLayout_6->addWidget(roomDepthHistogram);

		backgroundHistogram = new HistogramWidget(groupBox_4);
		backgroundHistogram->setObjectName("backgroundHistogram");
		backgroundHistogram->setMinimumSize(QSize(0, 64));

		verticalLayout_5->addWidget(backgroundHistogram);
        retranslateUi(MainWindow);
        QObject::connect(fileClose, &QAction::triggered, MainWindow, qOverload<>(&QMainWindow::close));

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MainWindow", nullptr));
        fileNew->setText(QCoreApplication::translate("MainWindow", "New", nullptr));
#if QT_CONFIG(shortcut)
        fileNew->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+N", nullptr));
#endif // QT_CONFIG(shortcut)
        fileOpen->setText(QCoreApplication::translate("MainWindow", "Open", nullptr));
#if QT_CONFIG(shortcut)
        fileOpen->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+O", nullptr));
#endif // QT_CONFIG(shortcut)
        fileSaveAs->setText(QCoreApplication::translate("MainWindow", "Save As", nullptr));
#if QT_CONFIG(shortcut)
        fileSaveAs->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+S", nullptr));
#endif // QT_CONFIG(shortcut)
        fileSave->setText(QCoreApplication::translate("MainWindow", "Save", nullptr));
#if QT_CONFIG(shortcut)
        fileSave->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+S", nullptr));
#endif // QT_CONFIG(shortcut)
        fileClose->setText(QCoreApplication::translate("MainWindow", "Close", nullptr));
#if QT_CONFIG(shortcut)
        fileClose->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+W", nullptr));
#endif // QT_CONFIG(shortcut)
        appExit->setText(QCoreApplication::translate("MainWindow", "Exit", nullptr));
        editUndo->setText(QCoreApplication::translate("MainWindow", "Undo", nullptr));
#if QT_CONFIG(shortcut)
        editUndo->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Z", nullptr));
#endif // QT_CONFIG(shortcut)
        editRedo->setText(QCoreApplication::translate("MainWindow", "Redo", nullptr));
#if QT_CONFIG(shortcut)
        editRedo->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+Z", nullptr));
#endif // QT_CONFIG(shortcut)
        selectAll->setText(QCoreApplication::translate("MainWindow", "Select All", nullptr));
#if QT_CONFIG(shortcut)
        selectAll->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+A", nullptr));
#endif // QT_CONFIG(shortcut)
        zoomIn->setText(QCoreApplication::translate("MainWindow", "Zoom In", nullptr));
#if QT_CONFIG(shortcut)
        zoomIn->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+=", nullptr));
#endif // QT_CONFIG(shortcut)
        zoomOut->setText(QCoreApplication::translate("MainWindow", "Zoom Out", nullptr));
#if QT_CONFIG(shortcut)
        zoomOut->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+-", nullptr));
#endif // QT_CONFIG(shortcut)
        zoomActual->setText(QCoreApplication::translate("MainWindow", "Actual Size", nullptr));
#if QT_CONFIG(shortcut)
        zoomActual->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+=", nullptr));
#endif // QT_CONFIG(shortcut)
        selectNone->setText(QCoreApplication::translate("MainWindow", "Select None", nullptr));
        editCut->setText(QCoreApplication::translate("MainWindow", "Cut", nullptr));
#if QT_CONFIG(shortcut)
        editCut->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+X", nullptr));
#endif // QT_CONFIG(shortcut)
        editCopy->setText(QCoreApplication::translate("MainWindow", "Copy", nullptr));
#if QT_CONFIG(shortcut)
        editCopy->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+C", nullptr));
#endif // QT_CONFIG(shortcut)
        editPaste->setText(QCoreApplication::translate("MainWindow", "Paste", nullptr));
#if QT_CONFIG(shortcut)
        editPaste->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+V", nullptr));
#endif // QT_CONFIG(shortcut)
        editDelete->setText(QCoreApplication::translate("MainWindow", "Delete", nullptr));
#if QT_CONFIG(shortcut)
        editDelete->setShortcut(QCoreApplication::translate("MainWindow", "Del", nullptr));
#endif // QT_CONFIG(shortcut)
        viewChecker->setText(QCoreApplication::translate("MainWindow", "Transparency Texture", nullptr));
        viewRooms->setText(QCoreApplication::translate("MainWindow", "Rooms", nullptr));
        viewLinks->setText(QCoreApplication::translate("MainWindow", "Links", nullptr));
        aboutQt->setText(QCoreApplication::translate("MainWindow", "About Qt", nullptr));
        aboutAbout->setText(QCoreApplication::translate("MainWindow", "About", nullptr));
        aboutHelp->setText(QCoreApplication::translate("MainWindow", "Help", nullptr));
#if QT_CONFIG(shortcut)
        aboutHelp->setShortcut(QCoreApplication::translate("MainWindow", "F1", nullptr));
#endif // QT_CONFIG(shortcut)
        linkAdd->setText(QCoreApplication::translate("MainWindow", "Add Link", nullptr));
        linkRemove->setText(QCoreApplication::translate("MainWindow", "Remove Link", nullptr));
        viewHalls->setText(QCoreApplication::translate("MainWindow", "Halls", nullptr));
        toolNew->setText(QCoreApplication::translate("MainWindow", "Create Room (N)", nullptr));
#if QT_CONFIG(tooltip)
        toolNew->setToolTip(QCoreApplication::translate("MainWindow", "Create Room", nullptr));
#endif // QT_CONFIG(tooltip)
        toolExtrude->setText(QCoreApplication::translate("MainWindow", "Extrude (E)", nullptr));
#if QT_CONFIG(tooltip)
        toolExtrude->setToolTip(QCoreApplication::translate("MainWindow", "Extrude", nullptr));
#endif // QT_CONFIG(tooltip)
        toolSlice->setText(QCoreApplication::translate("MainWindow", "Slice", nullptr));
#if QT_CONFIG(tooltip)
        toolSlice->setToolTip(QCoreApplication::translate("MainWindow", "Slice (Ctrl+R)", nullptr));
#endif // QT_CONFIG(tooltip)
        musicMute->setText(QCoreApplication::translate("MainWindow", "Mute", nullptr));
#if QT_CONFIG(shortcut)
        musicMute->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+M", nullptr));
#endif // QT_CONFIG(shortcut)
        fileImportWalls->setText(QCoreApplication::translate("MainWindow", "Load Room Types", nullptr));
        fileLoadBackground->setText(QCoreApplication::translate("MainWindow", "Load Background", nullptr));
        musicLoad->setText(QCoreApplication::translate("MainWindow", "Load FMOD Bank", nullptr));
        toolTranslate->setText(QCoreApplication::translate("MainWindow", "Translate (G)", nullptr));
        toolRotate->setText(QCoreApplication::translate("MainWindow", "Rotate (R)", nullptr));
        toolScale->setText(QCoreApplication::translate("MainWindow", "Scale (S)", nullptr));
        fileSaveRooms->setText(QCoreApplication::translate("MainWindow", "Save Room System", nullptr));
        toolReseat->setText(QCoreApplication::translate("MainWindow", "Reseat", nullptr));
#if QT_CONFIG(shortcut)
        toolReseat->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+H", nullptr));
#endif // QT_CONFIG(shortcut)
        fileCAOS->setText(QCoreApplication::translate("MainWindow", "Export CAOS", nullptr));
        viewGravity->setText(QCoreApplication::translate("MainWindow", "Gravity Direction", nullptr));
        toolDuplicate->setText(QCoreApplication::translate("MainWindow", "Duplicate", nullptr));
#if QT_CONFIG(shortcut)
        toolDuplicate->setShortcut(QCoreApplication::translate("MainWindow", "Shift+D", nullptr));
#endif // QT_CONFIG(shortcut)
        toolFace->setText(QCoreApplication::translate("MainWindow", "Add Face (F)", nullptr));
#if QT_CONFIG(shortcut)
        toolFace->setShortcut(QCoreApplication::translate("MainWindow", "F", nullptr));
#endif // QT_CONFIG(shortcut)
        toolRealign->setText(QCoreApplication::translate("MainWindow", "Align Gravity With Slice", nullptr));
#if QT_CONFIG(shortcut)
        toolRealign->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+G", nullptr));
#endif // QT_CONFIG(shortcut)
        toolPropagate->setText(QCoreApplication::translate("MainWindow", "Set Gravity Direction", nullptr));
#if QT_CONFIG(shortcut)
        toolPropagate->setShortcut(QCoreApplication::translate("MainWindow", "Alt+G", nullptr));
#endif // QT_CONFIG(shortcut)
        selectInvert->setText(QCoreApplication::translate("MainWindow", "Invert Selection", nullptr));
#if QT_CONFIG(shortcut)
        selectInvert->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+I", nullptr));
#endif // QT_CONFIG(shortcut)
        selectConnected->setText(QCoreApplication::translate("MainWindow", "Connected", nullptr));
#if QT_CONFIG(shortcut)
        selectConnected->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+L", nullptr));
#endif // QT_CONFIG(shortcut)
        selectOverlap->setText(QCoreApplication::translate("MainWindow", "Overlapping", nullptr));
#if QT_CONFIG(shortcut)
        selectOverlap->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+L", nullptr));
#endif // QT_CONFIG(shortcut)
        selectRoomType->setText(QCoreApplication::translate("MainWindow", "By Room Type", nullptr));
#if QT_CONFIG(shortcut)
        selectRoomType->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+T", nullptr));
#endif // QT_CONFIG(shortcut)
        selectRoomMusic->setText(QCoreApplication::translate("MainWindow", "By Room Music", nullptr));
#if QT_CONFIG(shortcut)
        selectRoomMusic->setShortcut(QCoreApplication::translate("MainWindow", "Ctrl+Shift+M", nullptr));
#endif // QT_CONFIG(shortcut)
        debugTreeSymmetry->setText(QCoreApplication::translate("MainWindow", "Test Tree Symmetry", nullptr));
        debugDoorSymmetry->setText(QCoreApplication::translate("MainWindow", "Test Door Symmetry", nullptr));
        viewRoomId->setText(QCoreApplication::translate("MainWindow", "View Room IDs", nullptr));
        debugSelectRoom->setText(QCoreApplication::translate("MainWindow", "Select Room by ID", nullptr));
        debugDumpPermTable->setText(QCoreApplication::translate("MainWindow", "Dump Perm Table", nullptr));
        toolAutoReseat->setText(QCoreApplication::translate("MainWindow", "Auto Reseat", nullptr));
        layerBaseColor->setText(QCoreApplication::translate("MainWindow", "Base Color", nullptr));
        layerNormals->setText(QCoreApplication::translate("MainWindow", "Normals", nullptr));
        layerOcclusion->setText(QCoreApplication::translate("MainWindow", "Ambient Occlusion", nullptr));
        layerRoughness->setText(QCoreApplication::translate("MainWindow", "Metallic Roughness", nullptr));
        layerDepth->setText(QCoreApplication::translate("MainWindow", "Depth", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("MainWindow", "Door Properties", nullptr));
        Permeability->setText(QCoreApplication::translate("MainWindow", "Permeability", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("MainWindow", "Room Properties", nullptr));
        label_20->setText(QCoreApplication::translate("MainWindow", "Music", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Type", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "Gravity", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "Gravity Strength", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "Gravity Direction", nullptr));
        groupBox_6->setTitle(QCoreApplication::translate("MainWindow", "Shade", nullptr));
        label_9->setText(QCoreApplication::translate("MainWindow", "Ambient Shade", nullptr));
        label_7->setText(QCoreApplication::translate("MainWindow", "Shade Strength", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow", "Shade Direction", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("MainWindow", "Audio Properties", nullptr));
        label_13->setText(QCoreApplication::translate("MainWindow", "Reverb", nullptr));
        label_10->setText(QCoreApplication::translate("MainWindow", "Unassigned 1", nullptr));
        label_11->setText(QCoreApplication::translate("MainWindow", "Unassigned 2", nullptr));
        label_12->setText(QCoreApplication::translate("MainWindow", "Unassigned 3", nullptr));
        groupBox_7->setTitle(QCoreApplication::translate("MainWindow", "Depth", nullptr));
        roomDepthLabel->setText(QCoreApplication::translate("MainWindow", "0 to 64 meters", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("MainWindow", "Depth", nullptr));
        backgroundDepthLabel->setText(QCoreApplication::translate("MainWindow", "0 to 64 meters", nullptr));
        menuFile->setTitle(QCoreApplication::translate("MainWindow", "File", nullptr));
        menuEdit->setTitle(QCoreApplication::translate("MainWindow", "Edit", nullptr));
        menuZoom->setTitle(QCoreApplication::translate("MainWindow", "Zoom", nullptr));
        menuSelect_2->setTitle(QCoreApplication::translate("MainWindow", "Select", nullptr));
        menuView->setTitle(QCoreApplication::translate("MainWindow", "View", nullptr));
        menuAbout->setTitle(QCoreApplication::translate("MainWindow", "Help", nullptr));
        menuLinks->setTitle(QCoreApplication::translate("MainWindow", "Links", nullptr));
        menuTools->setTitle(QCoreApplication::translate("MainWindow", "Tools", nullptr));
        menuDebug->setTitle(QCoreApplication::translate("MainWindow", "Debug", nullptr));
        menuBackground_Layer->setTitle(QCoreApplication::translate("MainWindow", "Background Layer", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
