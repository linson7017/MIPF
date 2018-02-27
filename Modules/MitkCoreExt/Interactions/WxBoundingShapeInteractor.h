/*===================================================================

* \author Guo Chu
* \date Jan,2018

===================================================================*/
#ifndef WxBoundingShapeInteractor_h
#define WxBoundingShapeInteractor_h

#include <mitkDataInteractor.h>
#include <mitkEventConfig.h>
#include <mitkGeometry3D.h>
#include <mitkDataStorage.h>
#include <mitkDataNode.h>

#include "qf_config.h"

class vtkRenderer;
class vtkPolyData;

namespace mitk
{
  class QF_API WxBoundingShapeInteractor : public DataInteractor
  {
  public:
    mitkClassMacro(WxBoundingShapeInteractor, DataInteractor);
		itkFactorylessNewMacro(Self) itkCloneMacro(Self)

		void SetDataStorage(mitk::DataStorage* dataStorage);
		void SetRenderer(vtkRenderer* renderer);
    virtual void SetDataNode(DataNode *dataNode) override;
		vtkPolyData* GetCube();
		void Undo();
		void Redo();

	public:
		typedef mitk::Message<> ImplementEventType;
		ImplementEventType UndoEvent;
		ImplementEventType RedoEvent;

  protected:
    WxBoundingShapeInteractor();
    virtual ~WxBoundingShapeInteractor();

    virtual void ConnectActionsAndFunctions() override;
    virtual void DataNodeChanged() override;
    void HandlePositionChanged(const InteractionEvent *interactionEvent, Point3D &center);

    virtual bool CheckOverObject(const InteractionEvent *);
    virtual bool CheckOverHandles(const InteractionEvent *interactionEvent);

    virtual void SelectObject(StateMachineAction *, InteractionEvent *);
    virtual void DeselectObject(StateMachineAction *, InteractionEvent *);
    virtual void SelectHandle(StateMachineAction *, InteractionEvent *);
    virtual void TranslateObject(StateMachineAction *, InteractionEvent *);
    virtual void ScaleObject(StateMachineAction *, InteractionEvent *);
    virtual void InitInteraction(StateMachineAction *, InteractionEvent *interactionEvent);
    virtual void DeselectHandles(StateMachineAction *, InteractionEvent *interactionEvent);
    bool InitMembers(InteractionEvent *interactionEvent);

		virtual void Undo(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);
		virtual void Redo(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent);

  private:
		mitk::DataStorage* m_pDataStorage;
		vtkRenderer* m_pRenderer;
		mitk::DataNode::Pointer m_pCubeNode;
		bool m_bInitFlag;

    class Impl;
    Impl *m_Impl;

		void Init();
  };
}
#endif
