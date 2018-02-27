#ifndef SphereCropperView_h__ 
#define SphereCropperView_h__ 
 
#include "MitkPluginView.h" 
#include <QWidget>
#include "ui_WxSphereCropperControls.h"
#include "Interactions/WxSphereShapeInteractor.h"

class CutImplementation;
 
class SphereCropperView : public QWidget, public MitkPluginView
{  
	Q_OBJECT
public:   
    SphereCropperView(); 
    ~SphereCropperView();
    void CreateView() override;
    WndHandle GetPluginHandle() override;

    void CutFinished(vtkObject* obj);
protected slots:
	/*!
	* @brief Creates a new sphere object
	*/
	virtual void DoCreateNewSphereObject();

	/*!
	* @brief Updates current selection of the bounding object
	*/
	void OnDataSelectionChanged(const mitk::DataNode* node);
// 
 	void StartInteraction();
 	void EndInteraction();
	void InsideOutChanged(bool checked);
	void Cut();
	void Undo();
	void Redo();

private:
	Ui::WxSphereCropperControls m_Controls;
	mitk::WeakPointer<mitk::DataNode> m_Node;
	mitk::WxSphereShapeInteractor::Pointer m_SphereShapeInteractor;
	CutImplementation* m_pImplementation;

	//用于关联DataStorage中的结点消息与结点事件
	virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);

    mitk::DataNode::Pointer m_resultSurface;
};
#endif // SphereCropperView_h__ 