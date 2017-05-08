#ifndef IQF_MitkReference_h__
#define IQF_MitkReference_h__

const char QF_MitkMain_Reference[] = "QF_MitkMain_Reference";

class IQF_MitkReference
{
public:
	virtual const char* GetString(const char* szID, const char* szDef="") = 0;
    virtual void SetString(const char* szID, const char* szValue) = 0;
	virtual bool GetBool(const char* szID, bool bDef=false) = 0;
	virtual void SetBool(const char* szID, bool value) = 0;
};
#endif // IQF_MitkReference_h__