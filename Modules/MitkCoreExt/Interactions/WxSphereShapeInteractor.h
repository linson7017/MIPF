/*===================================================================

* \author Guo Chu
* \date Jan,2018

===================================================================*/

#ifndef WxSphereShapeInteractor_h__
#define WxSphereShapeInteractor_h__

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
	class QF_API WxSphereShapeInteractor : public DataInteractor
	{
	public:
		mitkClassMacro(WxSphereShapeInteractor, DataInteractor);
		itkFactorylessNewMacro(Self) itkCloneMacro(Self)

		void SetDataStorage(mitk::DataStorage* dataStorage);
		void SetRenderer(vtkRenderer* renderer);
		virtual void SetDataNode(DataNode *dataNode) override;
		vtkPolyData* GetSphere();
		void Undo();
		void Redo();

	public:
		typedef mitk::Message<> ImplementEventType;
		ImplementEventType UndoEvent;
		ImplementEventType RedoEvent;
        typedef mitk::Message2<vtkObject*, mitk::InteractionEvent *> InteractionImplementEventType;
        InteractionImplementEventType ProcessEvent;

	protected:
		WxSphereShapeInteractor();
		virtual ~WxSphereShapeInteractor();

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
		mitk::DataNode::Pointer m_pSphereNode;
		bool m_bInitFlag;

		class Impl;
		Impl *m_Impl;

		void Init();
	};
}

#endif // WxSphereShapeInteractor_h__