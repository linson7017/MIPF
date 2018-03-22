#include "SurfaceInteractionView.h"

#include "Interactions/GeometryInteractor.h"


#include <usModuleRegistry.h>
#include <usGetModuleContext.h>

SurfaceInteractionView::SurfaceInteractionView()
{
}


SurfaceInteractionView::~SurfaceInteractionView()
{
}

void SurfaceInteractionView::CreateView()
{
    m_ui.setupUi(this);
   
    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("helper object",mitk::BoolProperty::New(true))));

    m_ui.TransformMatrix->setReadOnly(true);

    connect(m_ui.StartBtn, &QPushButton::clicked, this, &SurfaceInteractionView::Start);
    connect(m_ui.ResetBtn, &QPushButton::clicked, this, &SurfaceInteractionView::Reset);
}

void SurfaceInteractionView::Reset()
{
       if (m_Interactor)
       {
          // static_cast<GeometryInteractor*>(m_Interactor.GetPointer())->Reset();
           RequestRenderWindowUpdate();
       }
}

void SurfaceInteractionView::Start(bool start)
{
    if (!m_ui.DataSelector->GetSelectedNode())
    {
        return;
    }
     if (start)
     {
         m_Interactor = m_ui.DataSelector->GetSelectedNode()->GetDataInteractor();
         if (m_Interactor.IsNull())
         {
             std::string configPath = m_pMain->GetConfigPath();
             m_Interactor = GeometryInteractor::New();
             m_Interactor->LoadStateMachine("AffineInteraction3D.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt"));
             m_Interactor->SetEventConfig("AffineMouseConfig.xml", us::ModuleRegistry::GetModule("MitkDataTypesExt")); 

             /*static_cast<GeometryInteractor*>(m_Interactor.GetPointer())->TransformChangedEvent.AddListener(
                 mitk::MessageDelegate1<SurfaceInteractionView,vtkMatrix4x4*>(this, &SurfaceInteractionView::ImageMatrixChanged));;*/

         }
         m_Interactor->SetDataNode(m_ui.DataSelector->GetSelectedNode());
         m_ui.DataSelector->GetSelectedNode()->SetBoolProperty("pickable", true);   //一定要设置
         m_ui.DataSelector->GetSelectedNode()->SetDataInteractor(m_Interactor);
     }
     else
     {
         m_Interactor = NULL;
         m_ui.DataSelector->GetSelectedNode()->SetBoolProperty("pickable", false);
         m_ui.DataSelector->GetSelectedNode()->SetDataInteractor(nullptr);
     }
}

void  SurfaceInteractionView::ImageMatrixChanged(vtkMatrix4x4* matrix)
{
    QString matrixStr = QString("%1 %2 %3 %4\n%5 %6 %7 %8\n%9 %10 %11 %12\n%13 %14 %15 %16")
        .arg(matrix->GetElement(0, 0)).arg(matrix->GetElement(0, 1)).arg(matrix->GetElement(0, 2)).arg(matrix->GetElement(0, 3))
        .arg(matrix->GetElement(1, 0)).arg(matrix->GetElement(1, 1)).arg(matrix->GetElement(1, 2)).arg(matrix->GetElement(1, 3))
        .arg(matrix->GetElement(2, 0)).arg(matrix->GetElement(2, 1)).arg(matrix->GetElement(2, 2)).arg(matrix->GetElement(2, 3))
        .arg(matrix->GetElement(3, 0)).arg(matrix->GetElement(3, 1)).arg(matrix->GetElement(3, 2)).arg(matrix->GetElement(3, 3));
    m_ui.TransformMatrix->setPlainText(matrixStr);

}
