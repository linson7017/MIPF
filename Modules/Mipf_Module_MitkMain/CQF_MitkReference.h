#ifndef CQF_MitkReference_h__
#define CQF_MitkReference_h__

#pragma once

#include "MitkMain/IQF_MitkReference.h"

class QSettings;
namespace QF {
    class IQF_Main;
}

class CQF_MitkReference : public IQF_MitkReference
{
public:
    CQF_MitkReference(QF::IQF_Main* pMain);
    ~CQF_MitkReference();
    virtual const char* GetString(const char* szID, const char* szDef="");
    virtual void SetString(const char* szID, const char* szValue);

	virtual bool GetBool(const char* szID, bool bDef=false);
	virtual void SetBool(const char* szID, bool value );

    virtual int GetInt(const char* szID, int bDef = 0);
    virtual void SetInt(const char* szID, int value);

    virtual double GetDouble(const char* szID, double bDef = 0);
    virtual void SetDouble(const char* szID, double value);
private:
    QSettings* m_Settings;
    QF::IQF_Main* m_pMain;
};

#endif // CQF_MitkReference_h__
