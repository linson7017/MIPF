#include "CQF_VesselToolsCom.h"
#include <string>
#include <assert.h>

#include  "VesselSegmentation.h"
#include "CenterLineExtraction.h"
#include "Core/IQF_ObjectFactory.h"

#include "internal/qf_interfacedef.h"

#include "iqf_main.h"

QF::IQF_Component* QF::QF_CreateComponentObject(QF::IQF_Main* pMain)
{
    QF::IQF_Component* pComponent = new CQF_VesselToolsCom(pMain);
    assert(pComponent);
    return pComponent;
}

CQF_VesselToolsCom::CQF_VesselToolsCom(QF::IQF_Main* pMain) :m_pMain(pMain)
{

}

CQF_VesselToolsCom::~CQF_VesselToolsCom()
{
    
    
}

void CQF_VesselToolsCom::Release()
{
    delete this;
}

bool CQF_VesselToolsCom::Init()
{
    IQF_ObjectFactory* pObjectFactory = (IQF_ObjectFactory*)m_pMain->GetInterfacePtr(QF_Core_ObjectFactory);
    if (pObjectFactory)
    {
        pObjectFactory->Register(Object_ID_VesselSegmentationTool, NEW_INSTANCE(VesselSegmentation));
        pObjectFactory->Register(Object_ID_CenterLineExtraction, NEW_INSTANCE(CenterLineExtraction));
    }   
    return true;
}

int CQF_VesselToolsCom::GetInterfaceCount()
{
    return 0;
}

const char* CQF_VesselToolsCom::GetInterfaceID(int iID)
{
    switch (iID)
    {
    default:
        break;
    }
    return "";
}

void* CQF_VesselToolsCom::GetInterfacePtr(const char* szInterfaceID)
{
     return NULL;
}