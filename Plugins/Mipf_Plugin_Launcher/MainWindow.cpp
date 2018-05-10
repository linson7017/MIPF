#include "MainWindow.h"

#include <QtWidgets>
#include <QtCore>
#include <QtGui>

#include "Res/R.h"
#include "Common/app_env.h"
#include "Common/qt_context.h"
#include "Utils/variant.h"

#include "MitkMain/IQF_MitkIO.h"
#include "MitkMain/IQF_MitkReference.h"
#include "MitkMain/IQF_MitkInit.h"


#include "SplashWindow.h"
#include "iqf_main.h"

MainWindow::MainWindow(const char* xmlfile, SplashWindow* pSplashWindow) :m_pSplashWindow(pSplashWindow)
{
    m_pMain = (QF::IQF_Main*)app_env::getMainPtr();
    m_pMain->Attach(this);
    SetupWidgets(xmlfile);
}


MainWindow::~MainWindow()
{
}

void MainWindow::Update(const char* szMessage, int iValue, void* pValue)
{

}

void MainWindow::SetupWidgets(const char* xmlfile)
{
    IQF_MitkInit* pMitkInit = (IQF_MitkInit*)m_pMain->GetInterfacePtr(QF_MitkMain_Init);
    pMitkInit->Init(nullptr);

    if (m_pSplashWindow)
    {
        m_pSplashWindow->showMessage("Startup window ...");
    }
    setContentView(xmlfile);
    
    R::Instance()->Constructed();
}