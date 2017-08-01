#include "CQF_MitkDisplayOption.h"

#include "MitkMain/IQF_MitkDataManager.h"

#include "mitkDataNode.h"

#include "iqf_main.h"

CQF_MitkDisplayOption::CQF_MitkDisplayOption(QF::IQF_Main* pMain) :m_pMain(pMain)
{
    m_Manager = mitk::LevelWindowManager::New();
}


CQF_MitkDisplayOption::~CQF_MitkDisplayOption()
{
}

void CQF_MitkDisplayOption::SetLevelWindow(mitk::DataNode* pNode, double dLevel, double dWindow)
{
    IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    if (!pDataManager)
    {
        return;
    }
    if (!m_Manager->GetDataStorage())
    {
        m_Manager->SetDataStorage(pDataManager->GetDataStorage());
    }
    typedef mitk::DataStorage::SetOfObjects NodeSetType;
    NodeSetType::ConstPointer nodes = pDataManager->GetDataStorage()->GetAll();
    NodeSetType::ConstIterator it = nodes->Begin();
    while (it != nodes->End())
    {
        mitk::DataNode::Pointer node = it.Value();
        if (node== pNode)
        {
            node->SetBoolProperty("imageForLevelWindow", true);
        }
        else
        {
            node->SetBoolProperty("imageForLevelWindow", false);
        }
        ++it;
    }
    mitk::LevelWindowProperty::Pointer levelWindowProperty =
        dynamic_cast<mitk::LevelWindowProperty *>(pNode->GetProperty("levelwindow"));
    mitk::LevelWindow levelwindow;
    levelwindow.SetLevelWindow(dLevel, dWindow);
    m_Manager->SetLevelWindowProperty(levelWindowProperty);
    m_Manager->SetLevelWindow(levelwindow);
}

