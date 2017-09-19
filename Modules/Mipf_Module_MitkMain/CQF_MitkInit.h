#ifndef QF_MitkInit_h__
#define QF_MitkInit_h__

#include "MitkMain/IQF_MitkInit.h"

namespace QF
{
    class IQF_Main;
}

class CQF_MitkInit : public IQF_MitkInit
{
public:
    CQF_MitkInit(QF::IQF_Main* pMain);
    ~CQF_MitkInit();

    virtual void Init(mitk::DataStorage* dataStorage = nullptr);

private:
    QF::IQF_Main* m_pMain;
};

#endif // QF_MitkInit_h__
