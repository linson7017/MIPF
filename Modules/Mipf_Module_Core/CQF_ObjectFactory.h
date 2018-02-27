#ifndef CQF_ObjectFactory_h__
#define CQF_ObjectFactory_h__

#include "Core/IQF_ObjectFactory.h"
#include "ObjectFactory.h"
#include <map>

#pragma once
class CQF_ObjectFactory:public IQF_ObjectFactory
{
public:
    typedef  std::map< std::string, ContructFunctionType > FunctionMapType;
    CQF_ObjectFactory();
    ~CQF_ObjectFactory();
    virtual void Register(const char* szName, ContructFunctionType tFunc)
    {
        m_classMap[szName] = tFunc;
    }
    virtual void *CreateObject(const char* szName);
    virtual int GetNumberOfRegisteredClass()
    {
        return (int)m_classMap.size();
    }

    virtual void Release() { delete this; }
private:
    FunctionMapType m_classMap;
};

#endif // CQF_ObjectFactory_h__
