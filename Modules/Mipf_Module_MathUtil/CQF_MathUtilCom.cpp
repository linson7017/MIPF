#include "CQF_MathUtilCom.h"
#include <string>
#include <assert.h>
#include <MathUtil.h>

QF::IQF_Component* QF::QF_CreateComponentObject(QF::IQF_Main* pMain)
{
    QF::IQF_Component* pComponent = new CQF_MathUtil;
    assert(pComponent);
    return pComponent;
}

CQF_MathUtil::CQF_MathUtil()
{

}


CQF_MathUtil::~CQF_MathUtil()
{
}


void CQF_MathUtil::Release()
{
    delete m_mathUtil;
    delete this;
}

bool CQF_MathUtil::Init()
{
    m_mathUtil = new MathUtil;
    return true;
}


int CQF_MathUtil::GetInterfaceCount()
{
    return 1;

}

const char* CQF_MathUtil::GetInterfaceID(int iID)
{

    switch (iID)
    {
    case 0:
        return QF_Algorithm_MathUtil;
    default:
        break;
    }
    return "";
}

void* CQF_MathUtil::GetInterfacePtr(const char* szInterfaceID)
{
    if (strcmp(szInterfaceID, QF_Algorithm_MathUtil) == 0)
    {
        return m_mathUtil;
    }
    else
        return NULL;
}