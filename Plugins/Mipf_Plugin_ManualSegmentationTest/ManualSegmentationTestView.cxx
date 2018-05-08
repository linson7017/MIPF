#include "ManualSegmentationTestView.h"
#include "iqf_main.h"
#include "Res/R.h"

#include <QUuid>

#include "mitkSurface.h"
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkLabelSetImage.h>
#include <mitkContourModel.h>
#include <mitkContourModelSet.h>
#include <mitkToolManager.h>
#include <mitkDrawPaintbrushTool.h>

#include "QmitkDataStorageComboBox.h"
#include "QmitkNewSegmentationDialog.h"

#include "MitkSegmentation/IQF_MitkSegmentationTool.h"
#include "MitkSegmentation/mitk_seg_msg.h"
#include "MitkMain/mitk_main_msg.h"


ManualSegmentationTestView::ManualSegmentationTestView(QF::IQF_Main* pMain) :MitkPluginView(pMain),
m_bToolInited(false),
m_pMitkSegmentationTool(NULL),
m_currentToolName("")
{
    m_pMain->Attach(this);
    m_pMitkSegmentationTool = (IQF_MitkSegmentationTool*)m_pMain->GetInterfacePtr(QF_MitkSegmentation_Tool);
}

void ManualSegmentationTestView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "MITK_MESSAGE_MST_CREATE_SEGMENT") == 0)
    {
        //do what you want for the message
         
         QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog();
         int dialogReturnValue = dialog->exec();
         if (dialogReturnValue == QDialog::Rejected) return;

         mitk::Color color = dialog->GetColor();
         mitk::DataNode::Pointer newNode = mitk::DataNode::New();
         m_pMitkSegmentationTool->CreateSegmentationNode(m_pImageSelector->GetSelectedNode(), newNode,
             dialog->GetSegmentationName().toStdString().c_str(), SegRGBColor(color.GetRed(), color.GetGreen(), color.GetBlue()));
         GetDataStorage()->Add(newNode, m_pImageSelector->GetSelectedNode());
         GetMitkRenderWindowInterface()->Reinit(newNode);
         m_pSegmentSelector->SetSelectedNode(newNode);

         if (!m_bToolInited)
         {
             m_pMitkSegmentationTool->Initialize();
             m_pMitkSegmentationTool->SetReferenceData(m_pImageSelector->GetSelectedNode());
             m_bToolInited = true;
         }
         m_pMitkSegmentationTool->SetWorkingData(newNode); 
         m_pMitkSegmentationTool->SetSurfaceInterpolateOn(GetGuiProperty("ManualSegmentationTest.Interpolate3D","checked").toBool());
    }
    else if(strcmp(szMessage, "MITK_MESSAGE_MST_CHANGETOOL") == 0)
    {
        QObject* obj = (QObject*)pValue;
        if (obj&&m_bToolInited)
        {
            if (m_currentToolName== obj->property("toolName").toString())
            {

                obj->setProperty("checked", false);
                m_currentToolName = "";
                m_pMitkSegmentationTool->ChangeTool("");
                return;
            }
            m_currentToolName = obj->property("toolName").toString();
            mitk::Tool* tool = m_pMitkSegmentationTool->ChangeTool(m_currentToolName.toStdString().c_str());
            m_pMitkSegmentationTool->SetReferenceData(m_pImageSelector->GetSelectedNode());
            m_pMitkSegmentationTool->SetWorkingData(m_pSegmentSelector->GetSelectedNode());
            if (tool)
            {
                std::cout << "Change To Tool " << m_currentToolName.toStdString() << std::endl;
            }
            m_pMain->SendMessageQf("MITK_MESSAGE_MST_PENSIZE_CHANGED", 0, R::Instance()->getObjectFromGlobalMap("ManualSegmentationTest.PenSize"));
        }
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_MST_PENSIZE_CHANGED") == 0)
    { 
        QObject* obj = (QObject*)pValue;
        if (obj)
        {
            if (obj->property("value").isValid())
            {
                if (m_currentToolName == "Paint" || m_currentToolName == "Wipe")
                {
                    mitk::DrawPaintbrushTool* paintTool = dynamic_cast<mitk::DrawPaintbrushTool*>(m_pMitkSegmentationTool->GetActivedTool());
                    if (paintTool)
                    {
                        paintTool->SetSize(obj->property("value").toInt());
                    }
                }
                SetGuiProperty("ManualSegmentationTest.PenSizeValue", "text", QString("%1").arg(obj->property("value").toInt()));
            }     
        }   
    }
    else if(strcmp(szMessage, "MITK_MESSAGE_MST_INTERPOLATE3D") == 0)
    {
        QObject* obj = (QObject*)pValue;
        if (obj&&obj->property("checked").isValid())
        {
            if (obj->property("checked").toBool())
            {
                for (DataNodeMapType::iterator it = m_surfaceNodes.begin();it!=m_surfaceNodes.end();it++)
                {
                    auto isSurfaceParent = mitk::NodePredicateProperty::New("surfaceUuid",mitk::StringProperty::New(it->first));
                    mitk::DataStorage::SetOfObjects::ConstPointer parentNodes = GetDataStorage()->GetSubset(isSurfaceParent);
                    if (parentNodes->Size()>0)
                    {
                        GetDataStorage()->Add(it->second,
                            parentNodes->ElementAt(0));
                    }               
                }            
                m_pMitkSegmentationTool->SetSurfaceInterpolateOn(true);
            }
            else
            {
                for (DataNodeMapType::iterator it = m_surfaceNodes.begin(); it != m_surfaceNodes.end(); it++)
                {
                    GetDataStorage()->Remove(it->second);
                }   
                m_pMitkSegmentationTool->SetSurfaceInterpolateOn(false);
            }
        }
    }
    else if (strcmp(szMessage, MITK_MESSAGE_NODE_REMOVED) == 0)
    {
        mitk::DataNode* node = (mitk::DataNode*)pValue; 
        if (!node)
        {
            return;
        }
        std::string name = node->GetName();
        std::string uuid;
        if (node->GetStringProperty("surfaceUuid", uuid))
        {
            GetDataStorage()->Remove(m_surfaceNodes[uuid]);
            m_surfaceNodes.erase(uuid);
        }     
    }
    else if (strcmp(szMessage, MITK_MESSAGE_SURFACE_INTERPOLATION_FINISHED) == 0)
    {
        if (iValue)
        {
            mitk::Surface* surface = (mitk::Surface*)pValue;
            std::string uuid;
            if (!m_pSegmentSelector->GetSelectedNode()->GetStringProperty("surfaceUuid", uuid))
            {
                uuid = QUuid::createUuid().toString().toStdString();
                m_pSegmentSelector->GetSelectedNode()->SetStringProperty("surfaceUuid", uuid.c_str());
                mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
                surfaceNode->SetData(surface);
                surfaceNode->SetName("surface");
                float color[3] = { 1.0,1.0,0.0 };
                m_pSegmentSelector->GetSelectedNode()->GetColor(color);
                surfaceNode->SetColor(color);
                GetDataStorage()->Add(surfaceNode, m_pSegmentSelector->GetSelectedNode());
                m_surfaceNodes[uuid] = surfaceNode;
            }
            else if (m_surfaceNodes.count(uuid)!=0)
            {
                m_surfaceNodes[uuid]->SetData(surface);
            }
            RequestRenderWindowUpdate();
        }
    }
}

void ManualSegmentationTestView::Constructed()
{
    m_pImageSelector = (QmitkDataStorageComboBox*)R::Instance()->getObjectFromGlobalMap("ManualSegmentationTest.ImageSelector");
    if (m_pImageSelector)
    {
        m_pImageSelector->SetDataStorage(GetDataStorage());
        m_pImageSelector->SetPredicate(CreateImagePredicate());
        connect(m_pImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnImageSelectionChanged(const mitk::DataNode *)));
    }
    

    m_pSegmentSelector = (QmitkDataStorageComboBox*)R::Instance()->getObjectFromGlobalMap("ManualSegmentationTest.SegmentResultSelector");
    if (m_pSegmentSelector)
    {
        m_pSegmentSelector->SetDataStorage(GetDataStorage());
        m_pSegmentSelector->SetPredicate(CreateImagePredicate());
        connect(m_pSegmentSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnSegmentSelectionChanged(const mitk::DataNode *)));
    }  
}

void ManualSegmentationTestView::OnImageSelectionChanged(const mitk::DataNode* node)
{
    if (!node)
    {
        return;
    }
    if (m_bToolInited&&m_pMitkSegmentationTool)
    {
        m_pMitkSegmentationTool->SetReferenceData(node);
    }
}
void ManualSegmentationTestView::OnSegmentSelectionChanged(const mitk::DataNode* node)
{
    if (!node)
    {
        return;
    }
    if (m_bToolInited&&m_pMitkSegmentationTool)
    {
        m_pMitkSegmentationTool->SetWorkingData(node);
    }
}