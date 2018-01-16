/********************************************************************
	FileName:    CutImplementation.h
	Author:        Ling Song
	Date:           Month 1 ; Year 2018
	Purpose:	     
*********************************************************************/
#ifndef CutImplementation_h__
#define CutImplementation_h__

#include "mitkDataNode.h"
#include "vtkPoints.h"
#include "vtkSmartPointer.h"
#include "vtkDataObject.h"
#include <stack>
#include "qf_config.h"

class QF_API CutImplementation
{
public:
    CutImplementation();
    virtual ~CutImplementation()
    {
    }
    void Cut(vtkObject* pCutData, mitk::InteractionEvent * interactionEvent);
    void SetInsideOut(bool b) { InsideOut = b; }

    void Init(mitk::DataNode* pDataNode);
    void Finished();
    void Reset();
    void Undo();
    void Redo();


    void Release();

    bool InsideOut;
    bool InitFlag;
protected:
    //implement by  subclass
    virtual vtkSmartPointer<vtkDataObject> CutImpl(vtkObject* pCutData, mitk::InteractionEvent * interactionEvent) = 0;
    virtual vtkDataObject* GetDataObject() = 0;
    virtual vtkSmartPointer<vtkDataObject> GetCopyOfDataObject() = 0;
    virtual void Refresh()=0;

    //protected functions
    const vtkSmartPointer<vtkDataObject> PopUndo();
    const vtkSmartPointer<vtkDataObject> PopRedo();
    const vtkSmartPointer<vtkDataObject> TopOfUndo();
    const vtkSmartPointer<vtkDataObject> TopOfRedo();
    void PushUndo(const vtkSmartPointer<vtkDataObject>& data);
    void PushRedo(const vtkSmartPointer<vtkDataObject>& data);
    void ClearUndo();
    void ClearRedo();

protected:
    mitk::DataNode::Pointer m_pDataNode;
    typedef std::stack< vtkSmartPointer<vtkDataObject> > DataList;
    DataList m_undoList;
    DataList  m_redoList;
};

#endif // CutImplementation_h__
