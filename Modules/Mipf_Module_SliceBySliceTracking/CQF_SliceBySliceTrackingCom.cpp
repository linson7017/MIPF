#include "CQF_SliceBySliceTrackingCom.h"
#include <string>
#include <assert.h>

QF::IQF_Component* QF::QF_CreateComponentObject(QF::IQF_Main* pMain)
{
    QF::IQF_Component* pComponent = new CQF_SliceBySliceTracking;
    assert(pComponent);
    return pComponent;
}

CQF_SliceBySliceTracking::CQF_SliceBySliceTracking()
{

}


CQF_SliceBySliceTracking::~CQF_SliceBySliceTracking()
{
}


void CQF_SliceBySliceTracking::Release()
{
    delete this;
}

bool CQF_SliceBySliceTracking::Init()
{
    m_pSliceBySliceTracing = new SliceBySliceBlobTracking;
    return true;
}


int CQF_SliceBySliceTracking::GetInterfaceCount()
{
    return 1;

}

const char* CQF_SliceBySliceTracking::GetInterfaceID(int iID)
{

    switch (iID)
    {
    case 0:
        return QF_Algorithm_SliceBySliceTracking;
    default:
        break;
    }
    return "";
}

void* CQF_SliceBySliceTracking::GetInterfacePtr(const char* szInterfaceID)
{
    if (strcmp(szInterfaceID, QF_Algorithm_SliceBySliceTracking) == 0)
    {
        return m_pSliceBySliceTracing;
    }
    else
        return NULL;
}