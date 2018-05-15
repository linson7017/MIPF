#ifndef PointList_h__
#define PointList_h__
#include "mitkDataNode.h"
#include "mitkPointSet.h"
#include "QmitkStdMultiWidget.h"
#include "mitkSliceNavigationController.h"

#include "MitkStd/IQF_MitkPointList.h"
namespace QF
{
    class IQF_Main;
}

namespace QF
{
    class IQF_Subject;
}

class PointListFactory : public IQF_PointListFactory
{
public:
    PointListFactory(QF::IQF_Main* pMain):m_pMain(pMain){}
    ~PointListFactory() {}
    IQF_MitkPointList* CreatePointList();
private:
    QF::IQF_Main* m_pMain;
};

class PointList:public IQF_MitkPointList
{
public:
    PointList(QF::IQF_Main* pMain);
    ~PointList();
    virtual void Initialize();
    virtual mitk::DataNode::Pointer CreateNewPointSetNode();
    virtual void SetPointSetNode(mitk::DataNode *newNode);
    virtual void AddPoint(bool bAdd = true);
    virtual void SetSingleMode(bool bSingleMode = true);
    virtual mitk::PointSet* GetPointSet();
    virtual void SavePoints();
    virtual mitk::PointSet::Pointer LoadPoints();
    virtual void Attach(QF::IQF_Observer* observer);
    virtual void Detach(QF::IQF_Observer* observer);
    virtual bool InsertPoint(const double x, const double y, const double z);
    mitk::DataNode::Pointer GetPointSetNode();
    virtual void Release();
private:
    mitk::PointSet * CheckForPointSetInNode(mitk::DataNode *node) const;
    void OnPointSetChanged(const itk::EventObject &e);
    void OnPointSetDeleted(const itk::EventObject &e);
    void EnableCallback();
    void DisableCallback();

    mitk::DataNode::Pointer m_PointSetNode;
    int m_Orientation;
    mitk::DataInteractor::Pointer m_DataInteractor;
    int m_TimeStep;

    itk::ReceptorMemberCommand<PointList>::Pointer m_modCommand;
    itk::ReceptorMemberCommand<PointList>::Pointer m_delCommand;

    QF::IQF_Main* m_pMain;
    QF::IQF_Subject* m_pSubject;

    bool m_bAddingPoint;
    bool m_bSingleMode;

    unsigned int m_PointSetModifiedObserverTag;
    unsigned int m_PointSetDeletedObserverTag;
};

#endif // PointList_h__
