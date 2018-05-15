#ifndef IQF_MitkPointList_h__
#define IQF_MitkPointList_h__

#include "IQF_Object.h"
#include "mitkPointSet.h"
#include "mitkDataNode.h"

const char QF_MitkStd_PointListFactory[] = "QF_MitkStd_PointListFactory";

namespace QF
{
    class IQF_Observer;
}



const char MITK_MESSAGE_POINTLIST_CHANGED[] = "MITK_MESSAGE_POINTLIST_CHANGED";
const char MITK_MESSAGE_POINTLIST_REMOVED[] = "MITK_MESSAGE_POINTLIST_REMOVED";

class IQF_MitkPointList :public IQF_Object
{
public:
    virtual void Initialize() = 0;
    virtual mitk::DataNode::Pointer CreateNewPointSetNode() = 0;
    virtual void SetPointSetNode(mitk::DataNode *pPointSetNode) = 0;
    virtual void AddPoint(bool bAdd = true) = 0;
    virtual void SetSingleMode(bool bSingleMode = true)=0;
    virtual bool InsertPoint(const double dX, const double dY, const double dZ)=0;
    virtual mitk::PointSet* GetPointSet() = 0;
    virtual void SavePoints() = 0;
    virtual mitk::PointSet::Pointer LoadPoints() = 0;
    virtual void Attach(QF::IQF_Observer* pObserver) = 0;
    virtual void Detach(QF::IQF_Observer* pObserver) = 0;
    virtual void Release() = 0;
};

class IQF_PointListFactory
{
public:
    virtual IQF_MitkPointList* CreatePointList() = 0;
};


#endif // IQF_MitkPointList_h__
