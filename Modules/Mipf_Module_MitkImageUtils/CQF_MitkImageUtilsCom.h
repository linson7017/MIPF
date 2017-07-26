#ifndef CQF_MitkImageUtilsCom_h__
#define CQF_MitkImageUtilsCom_h__

#pragma once
#include "iqf_component.h"


class ImageCropper;

class CQF_MitkImageUtilsCom :public QF::IQF_Component
{
public:
    CQF_MitkImageUtilsCom(QF::IQF_Main* pMain);
    ~CQF_MitkImageUtilsCom();
    virtual void Release();
    virtual bool Init();
    virtual void* GetInterfacePtr(const char* szInterfaceID);
    const char* GetComponentID() { return "QF_Component_CQF_MitkImageUtilsCom"; }
    int GetInterfaceCount();
    const char* GetInterfaceID(int iID);
private:
    
    ImageCropper* m_pImageCropper;

   QF::IQF_Main* m_pMain;
};

#endif // CQF_MitkImageUtilsCom_h__