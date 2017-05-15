#ifndef CQF_MainCommand_h__
#define CQF_MainCommand_h__
#include "qf_config.h"
#include "iqf_command.h"

#include "mitkDataNode.h"
#include "mitkNodePredicateAnd.h"
#include "mitkNodePredicateOr.h"
#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateNot.h"

#pragma once

namespace QF {
	class IQF_Main;
}

class CQF_MainCommand :public QF::IQF_Command
{
public:
	CQF_MainCommand(QF::IQF_Main* pMain);
	~CQF_MainCommand();
	void Release();
	virtual bool ExecuteCommand(const char* szCommandID, QF::IQF_PropertySet* pInParam, QF::IQF_PropertySet* pOutParam);
	virtual int GetCommandCount();
	virtual const char* GetCommandID(int iIndex);
private:
	mitk::DataNode* CreateSegmentationNode();
    void ApplyDisplayOptions(mitk::DataNode* node);

    virtual void OnSelectionChanged(mitk::DataNode::Pointer node);
    virtual void OnSelectionChanged(std::vector<mitk::DataNode::Pointer> nodes);
    void SetToolManagerSelection(const mitk::DataNode* referenceData, const mitk::DataNode* workingData);
    void OnContourMarkerSelected(const mitk::DataNode* node);
    void RenderingManagerReinitialized();
    bool CheckForSameGeometry(const mitk::DataNode*, const mitk::DataNode*) const;


    void NodeAdded(const mitk::DataNode *node);

    mitk::NodePredicateOr::Pointer m_IsOfTypeImagePredicate;
    mitk::NodePredicateProperty::Pointer m_IsBinaryPredicate;
    mitk::NodePredicateNot::Pointer m_IsNotBinaryPredicate;
    mitk::NodePredicateAnd::Pointer m_IsNotABinaryImagePredicate;
    mitk::NodePredicateAnd::Pointer m_IsABinaryImagePredicate;

    mitk::NodePredicateOr::Pointer m_IsASegmentationImagePredicate;
    mitk::NodePredicateAnd::Pointer m_IsAPatientImagePredicate;

	QF::IQF_Main* m_pMain;
};

#endif // CQF_MainCommand_h__
