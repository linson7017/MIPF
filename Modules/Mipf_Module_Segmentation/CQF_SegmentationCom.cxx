#include "CQF_SegmentationCom.h"
#include <string>
#include <assert.h>

#include "CQF_SegmentationFactory.h"

#include "internal/qf_interfacedef.h"

QF::IQF_Component* QF::QF_CreateComponentObject(QF::IQF_Main* pMain)
{
    QF::IQF_Component* pComponent = new CQF_SegmentationCom(pMain);
    assert(pComponent);
    return pComponent;
}

CQF_SegmentationCom::CQF_SegmentationCom(QF::IQF_Main* pMain) :m_pMain(pMain)
{

}

CQF_SegmentationCom::~CQF_SegmentationCom()
{
	delete m_pSegmentationMethodFactory;
}

void CQF_SegmentationCom::Release()
{
	
    delete this;
}

bool CQF_SegmentationCom::Init()
{
    m_pSegmentationMethodFactory = new CQF_SegmentationFactory();
    return true;
}

int CQF_SegmentationCom::GetInterfaceCount()
{
    return 2;
}

const char* CQF_SegmentationCom::GetInterfaceID(int iID)
{
    switch (iID)
    {
    case 0:
        return QF_Segmentation_Factory;
    default:
        break;
    }
    return "";
}

void* CQF_SegmentationCom::GetInterfacePtr(const char* szInterfaceID)
{
    if (strcmp(szInterfaceID, QF_Segmentation_Factory) == 0)
    {
        return m_pSegmentationMethodFactory;
    }
    else
        return NULL;
}