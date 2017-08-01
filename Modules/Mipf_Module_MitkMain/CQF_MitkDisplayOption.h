#ifndef CQF_MitkDisplayOption_h__
#define CQF_MitkDisplayOption_h__

#include "mitkLevelWindowManager.h"
#include "MitkMain/IQF_MitkDisplayOption.h"

namespace QF
{
    class IQF_Main;
}

#pragma once
class CQF_MitkDisplayOption : public IQF_MitkDisplayOption
{
public:
    CQF_MitkDisplayOption(QF::IQF_Main* pMain);
    ~CQF_MitkDisplayOption();
    virtual void SetLevelWindow(mitk::DataNode* pNode,double dLevel,double dWindow);

private:
    mitk::LevelWindowManager::Pointer m_Manager;
    QF::IQF_Main* m_pMain;
};

#endif // CQF_MitkDisplayOption_h__
