#ifndef IQF_MitkReference_h__
#define IQF_MitkReference_h__


const char QF_MitkMain_Reference[] = "QF_MitkMain_Reference";

class IQF_MitkReference
{
public:
	virtual const char* GetStringConfig(const char* szID) = 0;
    virtual void SetStringConfig(const char* szID, const char* szValue) = 0;
};
#endif // IQF_MitkReference_h__