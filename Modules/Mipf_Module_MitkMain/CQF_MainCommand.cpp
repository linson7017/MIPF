#include "CQF_MainCommand.h"
#include "MitkMain/mitk_command_def.h"
#include <string.h>

//mitk
#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkRenderWindow.h"
#include "MitkMain/IQF_MitkReference.h"

#include "mitkRenderingManager.h"

#include "QmitkIOUtil.h"
#include <QFileDialog>

#include "iqf_main.h"

//qt
#include <QMessageBox>

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

bool CQF_MainCommand::ExecuteCommand(const char* szCommandID, QF::IQF_PropertySet* pInParam, QF::IQF_PropertySet* pOutParam)
{
    if (strcmp(szCommandID, MITK_MAIN_COMMAND_LOADDATA) == 0)
    {
        IQF_MitkReference* pMitkReference = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
        IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        IQF_MitkRenderWindow* pMitkRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
        QString defaultOpenFilePath = pMitkReference->GetString("LastOpenDirectory");

        QStringList fileNames = QFileDialog::getOpenFileNames(NULL, "Open",
            defaultOpenFilePath,
            QmitkIOUtil::GetFileOpenFilterString());
        if (fileNames.empty())
            return false;
        try
        {
            QmitkIOUtil::Load(fileNames, *pMitkDataManager->GetDataStorage());
        }
        catch (const mitk::Exception& e)
        {
            MITK_INFO << e;
            return false;
        }
        mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(pMitkDataManager->GetDataStorage());
        pMitkReference->SetString("LastOpenDirectory", defaultOpenFilePath.toStdString().c_str());
        return true;
    }
    else
    {
        return false;
    }
}

int CQF_MainCommand::GetCommandCount()
{
    return 1;
}

const char* CQF_MainCommand::GetCommandID(int iIndex)
{
    switch (iIndex)
    {
    case 0:
        return MITK_MAIN_COMMAND_LOADDATA;
    default:
        return "";
        break;
    }
}
