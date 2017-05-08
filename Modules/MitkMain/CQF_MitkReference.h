#ifndef CQF_MitkReference_h__
#define CQF_MitkReference_h__

#pragma once

#include "MitkMain/IQF_MitkReference.h"
#include <mitkStandaloneDataStorage.h>
#include <MitkException.h>
class QSettings;


class CQF_MitkReference : public IQF_MitkReference
{
public:
    CQF_MitkReference();
    ~CQF_MitkReference();
    virtual const char* GetString(const char* szID, const char* szDef="");
    virtual void SetString(const char* szID, const char* szValue);
	virtual bool GetBool(const char* szID, bool bDef=false);
	virtual void SetBool(const char* szID, bool value );
private:
    QSettings* m_Settings;
};

#endif // CQF_MitkReference_h__
