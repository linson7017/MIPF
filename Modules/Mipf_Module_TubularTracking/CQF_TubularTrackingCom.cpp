#include "CQF_TubularTrackingCom.h"
#include "TubularTracking.h"

QF::IQF_Component* QF::QF_CreateComponentObject(QF::IQF_Main* pMain)
{
    QF::IQF_Component* pComponent = new CQF_TubularTrackingCom;
    assert(pComponent);
    return pComponent;
}

CQF_TubularTrackingCom::CQF_TubularTrackingCom()
{
    
}

CQF_TubularTrackingCom::~CQF_TubularTrackingCom()
{
    delete m_pTubularTracing;
}

void CQF_TubularTrackingCom::Release()
{
    delete this;
}

bool CQF_TubularTrackingCom::Init()
{
    m_pTubularTracing = new TubularTracking;
    return true;
}


int CQF_TubularTrackingCom::GetInterfaceCount()
{
    return 1;

}

const char* CQF_TubularTrackingCom::GetInterfaceID(int iID)
{

    switch (iID)
    {
    case 0:
        return QF_Algorithm_TubularTracking;
    default:
        break;
    }
    return "";
}

void* CQF_TubularTrackingCom::GetInterfacePtr(const char* szInterfaceID)
{
    if (strcmp(szInterfaceID, QF_Algorithm_TubularTracking)==0)
    {
        return m_pTubularTracing;
    }
    else
        return NULL;
}
