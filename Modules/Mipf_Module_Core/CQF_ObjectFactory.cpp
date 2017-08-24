#include "CQF_ObjectFactory.h"



CQF_ObjectFactory::CQF_ObjectFactory()
{
}


CQF_ObjectFactory::~CQF_ObjectFactory()
{
}

void* CQF_ObjectFactory::CreateObject(const char* szName)
{
    FunctionMapType::const_iterator it;
    it = m_classMap.find(szName);
    if (it == m_classMap.end())
        return  0;
    else
        return it->second();  //func();
}
