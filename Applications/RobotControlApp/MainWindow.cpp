#include "MainWindow.h"

#include <QtWidgets>
#include <QtCore>
#include <QtGui>

#include "Res/R.h"
#include "Common/app_env.h"
#include "Utils/variant.h"

//hardware control
#include <QFile>
#include <QtMath>
#include <QTime>


#include <iostream>
#include "iqf_main.h"

MainWindow::MainWindow(const char* xmlfile)
{
    m_pMain = (QF::IQF_Main*)app_env::getMainPtr();
    m_pMain->Attach(this);
    setContentView(xmlfile);

    /*m_SpeedLineEdit = (QLineEdit*)R::Instance()->getObjectFromGlobalMap("main.Speed");

    m_MoveXBtn = (QPushButton*)R::Instance()->getObjectFromGlobalMap("main.MoveX");
    m_MoveYBtn = (QPushButton*)R::Instance()->getObjectFromGlobalMap("main.MoveY");
    m_MoveZBtn = (QPushButton*)R::Instance()->getObjectFromGlobalMap("main.MoveZ");

    connect(m_MoveXBtn, SIGNAL(pressed()), this, SLOT(SlotMoveX()));
    connect(m_MoveYBtn, SIGNAL(pressed()), this, SLOT(SlotMoveY()));
    connect(m_MoveZBtn, SIGNAL(pressed()), this, SLOT(SlotMoveZ()));*/
}


MainWindow::~MainWindow()
{
}

void MainWindow::Update(const char* szMessage, int iValue, void* pValue)
{
    
}
