#include "CQF_MainCommand.h"
#include "MitkMain/mitk_command_def.h"
#include <string.h>

//mitk
#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkRenderWindow.h"
#include "MitkMain/IQF_MitkReference.h"

#include "mitkRenderingManager.h"
#include "mitkToolManager.h"
#include <mitkToolManagerProvider.h>
#include "mitkApplicationCursor.h"
#include "mitkSegmentationObjectFactory.h"
#include "mitkCameraController.h"
#include "mitkLabelSetImage.h"

#include "QmitkIOUtil.h"
#include <QFileDialog>

#include "iqf_main.h"

//qt
#include <QMessageBox>

#include "QmitkNewSegmentationDialog.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkSegmentationOrganNamesHandling.cpp"

#include "usModuleResource.h"
#include "usModuleResourceStream.h"

#include "CQF_ManualSegmentation.h"



CQF_MainCommand::CQF_MainCommand(QF::IQF_Main* pMain)
{
    m_pMain = pMain;
}


CQF_MainCommand::~CQF_MainCommand()
{
}

void CQF_MainCommand::Release()
{
    delete this;
}

int CQF_MainCommand::GetCommandCount()
{
    return 0;
}

const char* CQF_MainCommand::GetCommandID(int iIndex)
{
    switch (iIndex)
    {
    case 0:
        return "";
    default:
        return "";
        break;
    }
}

bool CQF_MainCommand::ExecuteCommand(const char* szCommandID, QF::IQF_Properties* pInParam, QF::IQF_Properties* pOutParam)
{
    if (strcmp(szCommandID, "") == 0)
    {

    }
    else
    {
        return false;
    }
}
