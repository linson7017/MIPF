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
}


MainWindow::~MainWindow()
{
}

void MainWindow::Update(const char* szMessage, int iValue, void* pValue)
{
    
}
