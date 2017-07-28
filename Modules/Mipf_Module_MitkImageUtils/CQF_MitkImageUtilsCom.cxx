#include "CQF_MitkImageUtilsCom.h"
#include <string>
#include <assert.h>
#include "internal/qf_interfacedef.h"

#include "ImageCropper.h"

QF::IQF_Component* QF::QF_CreateComponentObject(QF::IQF_Main* pMain)
{
    QF::IQF_Component* pComponent = new CQF_MitkImageUtilsCom(pMain);
    assert(pComponent);
    return pComponent;
}

CQF_MitkImageUtilsCom::CQF_MitkImageUtilsCom(QF::IQF_Main* pMain) :m_pMain(pMain)
{

}

CQF_MitkImageUtilsCom::~CQF_MitkImageUtilsCom()
{
    
    
}

void CQF_MitkImageUtilsCom::Release()
{
    delete this;
}

bool CQF_MitkImageUtilsCom::Init()
{
    m_pImageCropper = new ImageCropper(m_pMain);
    
    return true;
}

int CQF_MitkImageUtilsCom::GetInterfaceCount()
{
    return 1;
}

const char* CQF_MitkImageUtilsCom::GetInterfaceID(int iID)
{
    switch (iID)
    {
    case 0:
        return QF_MitkImageUtils_ImageCropper;
    default:
        break;
    }
    return "";
}

void* CQF_MitkImageUtilsCom::GetInterfacePtr(const char* szInterfaceID)
{
    if (strcmp(szInterfaceID, QF_MitkImageUtils_ImageCropper) == 0)
    {
        return m_pImageCropper;
    }
    else
        return NULL;
}