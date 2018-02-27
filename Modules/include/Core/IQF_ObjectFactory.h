#ifndef IQF_ObjectFactory_h__
#define IQF_ObjectFactory_h__

#pragma once
#include <functional>
#include "IQF_Object.h"

const char QF_Core_ObjectFactory[] = "QF_Core_ObjectFactory";

typedef std::function<void*(void)> ContructFunctionType;

class IQF_ObjectFactory : public IQF_Object
{
public:
    virtual void Register(const char* szName, ContructFunctionType tFunc) = 0;
    virtual void *CreateObject(const char* szName) = 0;
    virtual int GetNumberOfRegisteredClass() = 0;
};

#define NEW_INSTANCE(TYPE) [](void) -> void * { return new TYPE();}

#endif // IQF_ObjectFactory_h__
