#include "CQF_CVACommand.h"
#include <string.h>
#include "iqf_main.h"

#include "CVA/cva_command_def.h"

#include "MitkMain/IQF_MitkReference.h"
#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkRenderWindow.h"
#include "CVA/IQF_DSATool.h"

#include <QFileDialog>
#include "QStringUtils.h"

//mitk
#include "mitkNodePredicateNot.h"
#include "mitkNodePredicateProperty.h"

#include "iqf_properties.h"
#include "iqf_property.h"

CQF_CVACommand::CQF_CVACommand(QF::IQF_Main* pMain)
{
    m_pMain = pMain;
}

CQF_CVACommand::~CQF_CVACommand()
{
}

void CQF_CVACommand::Release()
{
   delete this;
}

bool CQF_CVACommand::ExecuteCommand(const char* szCommandID, QF::IQF_Properties* pInParam, QF::IQF_Properties* pOutParam)
{
    if (strcmp(szCommandID, CVA_COMMAND_LOAD_DSA) == 0)
    {     
        std::string datastorageID = pInParam->GetStringProperty("DataStorage", "");
        IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        IQF_DSATool* pDSATool = (IQF_DSATool*)m_pMain->GetInterfacePtr(QF_INTERFACE_DSA_TOOL);
        if (!pDataManager||!pDSATool)
        {
            return false;
        }
        mitk::DataStorage::Pointer pDataStorage = pDataManager->GetDataStorage(datastorageID);
        if (!pDataStorage)
        {
            return false;
        }
        QString defaultOpenFilePath = "";
        IQF_MitkReference* pReference = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
        if (pReference)
        {
            defaultOpenFilePath = pReference->GetString("LastOpenDirectory");
        }
        QString fileName = QFileDialog::getOpenFileName(NULL, "Open Dicom",
            defaultOpenFilePath,
            "DICOM (*.*)");
        if (fileName.isEmpty())
            return false;
        try
        {
            QFileInfo fi(fileName);
            if (fi.exists())
            {
                std::string localStr;
                mitk::DataNode::Pointer node = pDSATool->LoadDSADicomFile(QStringUtils::GetLocalString(fi.absoluteFilePath(), localStr),
                                                                                                                    QStringUtils::GetLocalString(fi.fileName(), localStr));
                if (node.IsNotNull())
                {
                    if (pInParam->GetBoolProperty("SingleMode", false))
                    {
                        pDataStorage->Remove(pDataStorage->GetSubset(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object"))));
                    }
                    node->SetStringProperty("filename", fi.absoluteFilePath().toLocal8Bit().constData());
                    pDataStorage->Add(node);
                    IQF_MitkRenderWindow* pRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
                    if (pRenderWindow)
                    {
                        pRenderWindow->Reinit(node);
                    }
                    return true;
                }
                else
                {
                    return false;
                }
            }
        }
        catch (const mitk::Exception& e)
        {
            MITK_ERROR << e;
            return false;
        }
        if (pReference)
        {
            pReference->SetString("LastOpenDirectory", QFileInfo(fileName).absolutePath().toStdString().c_str());
        }
        return true;
    }
    else
    {
        return false;
    }
}

int CQF_CVACommand::GetCommandCount()
{
    return 1;
}

const char* CQF_CVACommand::GetCommandID(int iIndex)
{
    switch (iIndex)
    {
    case 0:
        return CVA_COMMAND_LOAD_DSA;
    default:
        return "";
        break;
    }
}

