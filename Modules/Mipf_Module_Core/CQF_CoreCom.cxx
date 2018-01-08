#include "CQF_CoreCom.h"
#include <string>
#include <assert.h>

#include "internal/qf_interfacedef.h"

#include "CQF_ObjectFactory.h"

QF::IQF_Component* QF::QF_CreateComponentObject(QF::IQF_Main* pMain)
{
    QF::IQF_Component* pComponent = new CQF_CoreCom(pMain);
    assert(pComponent);
    return pComponent;
}

CQF_CoreCom::CQF_CoreCom(QF::IQF_Main* pMain) :m_pMain(pMain)
{

}

CQF_CoreCom::~CQF_CoreCom()
{
    
    
}

void CQF_CoreCom::Release()
{
    delete m_pObjectFactory;
    delete this;
}

bool CQF_CoreCom::Init()
{
    
    m_pObjectFactory = new CQF_ObjectFactory;
    return true;
}

int CQF_CoreCom::GetInterfaceCount()
{
    return 1;
}

const char* CQF_CoreCom::GetInterfaceID(int iID)
{
    switch (iID)
    {
    case 0:
        return QF_Core_ObjectFactory;
    default:
        break;
    }
    return "";
}

void* CQF_CoreCom::GetInterfacePtr(const char* szInterfaceID)
{
    if (strcmp(szInterfaceID, QF_Core_ObjectFactory) == 0)
    {
        return m_pObjectFactory;
    }
    else
        return NULL;
}