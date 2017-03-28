#ifndef CQF_MitkReference_h__
#define CQF_MitkReference_h__

#pragma once

#include "MitkMain/IQF_MitkReference.h"
#include <mitkStandaloneDataStorage.h>
class QSettings;

class CQF_MitkReference : public IQF_MitkReference
{
public:
    CQF_MitkReference();
    ~CQF_MitkReference();
    virtual const char* GetStringConfig(const char* szID);
    virtual void SetStringConfig(const char* szID, const char* szValue);
private:
    QSettings* m_Settings;
};

#endif // CQF_MitkReference_h__
