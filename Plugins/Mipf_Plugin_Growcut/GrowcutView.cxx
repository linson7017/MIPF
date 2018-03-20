#include "GrowcutView.h"
#include "iqf_main.h"
#include "Res/R.h"
#include "Utils/variant.h"

#include <QPushButton>
#include <mitkImageCast.h>
#include <vtkImageShiftScale.h>
#include <vtkMath.h>
#include <vtkImageMathematics.h>
#include <vtkImageMedian3D.h>

#include <mitkToolManager.h>
#include <mitkToolManagerProvider.h>
#include <mitkDrawPaintbrushTool.h>
#include <mitkLabelSetImage.h>
#include <mitkSurfaceBasedInterpolationController.h>

//qmitk
#include <QmitkDataStorageComboBox.h>

#include "Segmentation/IQF_FastGrowCutSegmentation.h"
#include "Segmentation/IQF_SegmentationMethodFactory.h"

#include "MitkSegmentation/IQF_MitkSegmentationTool.h"

#include "iqf_properties.h"

GrowcutView::GrowcutView(QF::IQF_Main* pMain) :MitkPluginView(pMain), 
m_pSourceImageSelector(NULL), 
m_bPaintForeground(true) , 
m_bInited(false),
m_seedImageNode(NULL),
m_tool(NULL),
pFGC(NULL)
{
    m_pMain->Attach(this);
	connect(&m_watcher, &QFutureWatcher<void>::finished, this, &GrowcutView::GrowcutFinished);

	m_bInitializationFlag = false;
	PaintFlag = false;

    IQF_SegmentationMethodFactory* pFactory = (IQF_SegmentationMethodFactory*)m_pMain->GetInterfacePtr(QF_Segmentation_Factory);
    if (pFactory)
    {
        pFGC = pFactory->CreateFastGrowCutSegmentationMethod();
    }
    m_pMitkSegTool = (IQF_MitkSegmentationTool*)m_pMain->GetInterfacePtr(QF_MitkSegmentation_Tool);
}

GrowcutView::~GrowcutView()
{
     if (pFGC)
     {
         delete pFGC;
         pFGC = NULL;
     }
}

void GrowcutView::Constructed()
{
	m_pSourceImageSelector = (QmitkDataStorageComboBox*)R::Instance()->getObjectFromGlobalMap("GrowcutSegmentation.ImageSelector");
	if (m_pSourceImageSelector)
	{
        m_pSourceImageSelector->SetDataStorage(GetDataStorage());
        m_pSourceImageSelector->SetPredicate(CreatePredicate(Image));
	} 
}

void GrowcutView::Update(const char* szMessage, int iValue, void* pValue)
{
    if (strcmp(szMessage, "MITK_MESSAGE_GROWCUT_SEGMENT") == 0)
    {
		QPushButton* btn = (QPushButton*)R::Instance()->getObjectFromGlobalMap("GrowcutSegmentation.Segment");
		if (btn)
		{
			btn->setText("Please wait...");
		}
		m_future = QtConcurrent::run(this,&GrowcutView::DoSomeThing);
		m_watcher.setFuture(m_future);
    }
    else  if (strcmp(szMessage, "MITK_MESSAGE_GROWCUT_INIT") == 0) 
    {
        Init();
    }
	else if (strcmp(szMessage, "Growcut.Reset") == 0)
	{
		m_bInitializationFlag = false;
		PaintFlag = false;
		QPushButton* paintbtn = (QPushButton*)R::Instance()->getObjectFromGlobalMap("Growcut.Paint");
		paintbtn->setText("Background Now");
		QPushButton* segbtn = (QPushButton*)R::Instance()->getObjectFromGlobalMap("Growcut.Button");
		segbtn->setText("Growcut Segmentation");
		GetDataStorage()->Remove(GetDataStorage()->GetNamedNode("Result"));
	}
	else if (strcmp(szMessage, "Growcut.PaintChange") == 0)
	{
		QPushButton* btn = (QPushButton*)R::Instance()->getObjectFromGlobalMap("Growcut.Paint");
		if (PaintFlag==false)
		{
			PaintFlag = true;
			btn->setText("Foreground Now");
		}
		else
		{
			PaintFlag = false;
			btn->setText("Background Now");
		}
	}
    else if (strcmp(szMessage, "MITK_MESSAGE_GROWCUT_FOREGROUND") == 0)
    {
        VarientMap* vm = (VarientMap*)pValue;
        variant v = variant::GetVariant(*vm, "checked");
        if (v.getBool())
        {
            SwitchToForeground();
        }
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GROWCUT_BACKGROUND") == 0)
    {
        VarientMap* vm = (VarientMap*)pValue;
        variant v = variant::GetVariant(*vm, "checked");
        if (v.getBool())
        {
            SwitchToBackground();
        }
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GROWCUT_BEGIN_PAINT") == 0)
    {
        ChangeTool("Paint");
        if (m_bPaintForeground)
        {
            SwitchToForeground();
        }
        else
        {
            SwitchToBackground();
        }
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GROWCUT_BEGIN_WIPE") == 0)
    {
        ChangeTool("Wipe");
        if (m_bPaintForeground)
        {
            SwitchToForeground();
        }
        else
        {
            SwitchToBackground();
        }
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GROWCUT_END_PAINT") == 0)
    {
        ChangeTool("");
    }
    else if (strcmp(szMessage, "MITK_MESSAGE_GROWCUT_PENSIZE_CHANGED") == 0)
    {
        QF::IQF_Properties* ps = (QF::IQF_Properties*)pValue;
        if (m_tool)
        {
            m_tool->SetSize(ps->GetIntProperty("pensize", 10));
        }
    }
}

void GrowcutView::Init()
{
    if (m_pMitkSegTool)
    {
        //init seed image
        if (1)
        {
            GetDataStorage()->Remove(m_seedImageNode);
            m_seedImageNode = mitk::DataNode::New();
            m_pMitkSegTool->CreateLabelSetImageNode(m_pSourceImageSelector->GetSelectedNode(), m_seedImageNode, "seed");
            mitk::LabelSetImage* labelImage = dynamic_cast<mitk::LabelSetImage*>(m_seedImageNode->GetData());
            if (labelImage)
            {
                //add foreground label
                mitk::Label::Pointer foregroundLabel = mitk::Label::New();
                foregroundLabel->SetName("foreground");
                foregroundLabel->SetValue(1);
                float fc[3] = { 0.0,1.0,0.0 };
                foregroundLabel->SetColor(mitk::Color(fc));
                labelImage->GetActiveLabelSet()->AddLabel(foregroundLabel);

                //add background label
                mitk::Label::Pointer backgroundLabel = mitk::Label::New();
                backgroundLabel->SetName("background");
                backgroundLabel->SetValue(2);
                float bc[3] = { 0.0,0.0,1.0 };
                backgroundLabel->SetColor(mitk::Color(bc));
                labelImage->GetActiveLabelSet()->AddLabel(backgroundLabel);
            }

            GetDataStorage()->Add(m_seedImageNode);
        }
        //Init Tool
        if (m_tool)
        {
            return;
        }
        m_pMitkSegTool->Initialize();
        m_pMitkSegTool->SetWorkingData(m_seedImageNode);
        m_pMitkSegTool->SetReferenceData(m_pSourceImageSelector->GetSelectedNode());
        m_bInited = true;
    }
    m_bInitializationFlag = false;
       
}

void GrowcutView::ChangeTool(const char* toolName)
{
    if (!m_bInited) return;
    m_tool = dynamic_cast<mitk::PaintbrushTool*>(m_pMitkSegTool->ChangeTool(toolName));
    if (m_tool)
    {
        m_tool->SetSize(GetGuiProperty("GrowcutSegmentation.PenSize","value").toInt());
    }
}

void GrowcutView::SwitchToForeground()
{
    if (!m_seedImageNode)
    {
        return;
    }
    mitk::LabelSetImage* labelImage = dynamic_cast<mitk::LabelSetImage*>(m_seedImageNode->GetData());
    if (labelImage)
    {
        labelImage->GetActiveLabelSet()->SetActiveLabel(1);
        labelImage->Modified();
        mitk::SurfaceBasedInterpolationController *interpolator = mitk::SurfaceBasedInterpolationController::GetInstance();
        if (interpolator)
        {
            interpolator->SetActiveLabel(1);
        }
    }
    m_bPaintForeground = true;
}

void GrowcutView::SwitchToBackground()
{
    if (!m_seedImageNode)
    {
        return;
    }
    mitk::LabelSetImage* labelImage = dynamic_cast<mitk::LabelSetImage*>(m_seedImageNode->GetData());
    if (labelImage)
    {
        labelImage->GetActiveLabelSet()->SetActiveLabel(2);
        labelImage->Modified();
        mitk::SurfaceBasedInterpolationController *interpolator = mitk::SurfaceBasedInterpolationController::GetInstance();
        if (interpolator)
        {
            interpolator->SetActiveLabel(2);
        }
    }
    m_bPaintForeground = false;
}

void GrowcutView::GrowcutFinished()
{
	GetDataStorage()->Remove(GetDataStorage()->GetNamedNode("Result"));

    mitk::Image* mitkSeedImage = dynamic_cast<mitk::Image*>(m_seedImageNode->GetData());
    mitk::Image::Pointer mitkResultImage = mitk::Image::New();
    mitkResultImage->Initialize(mitkSeedImage);
    pFGC->GetForegroundmage(mitkResultImage->GetVtkImageData());

    mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
    resultNode->SetData(mitkResultImage);
    resultNode->SetName("Result");
	GetDataStorage()->Add(resultNode);

	QPushButton* btn = (QPushButton*)R::Instance()->getObjectFromGlobalMap("GrowcutSegmentation.Segment");
	btn->setText("Done Or Do Deeply");
	RequestRenderWindowUpdate();
}

void GrowcutView::DoSomeThing()
{	
	//得到原图像，并转换为vtk图像
	mitk::Image* mitkSourceImage = dynamic_cast<mitk::Image*>(m_pSourceImageSelector->GetSelectedNode()->GetData());
	
    mitk::Image* mitkSeedImage = dynamic_cast<mitk::Image*>(m_seedImageNode->GetData());

		//grow cut算法处理
	DoSegmentation(mitkSourceImage->GetVtkImageData(), mitkSeedImage->GetVtkImageData());
}

void GrowcutView::DoSegmentation(vtkImageData* SrcImage, vtkImageData* LabImage)
{
	if (pFGC)
	{
		pFGC->SetSourceImage(SrcImage);
		pFGC->SetSeedlImage(LabImage);
		//第一次对图像进行粗分割，请将m_bInitializationFlag设置为false
		pFGC->SetWorkMode(m_bInitializationFlag);
		pFGC->Init();
		pFGC->DoSegmentation();
		m_bInitializationFlag = true;
	}
}