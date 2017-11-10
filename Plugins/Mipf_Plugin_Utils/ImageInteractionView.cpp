#include "ImageInteractionView.h"

#include "Interactions/ImageInteractor.h"


ImageInteractionView::ImageInteractionView()
{
}


ImageInteractionView::~ImageInteractionView()
{
}

void ImageInteractionView::CreateView()
{
    m_ui.setupUi(this);
   
    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Image>::New());

    m_ui.TransformMatrix->setReadOnly(true);

    connect(m_ui.StartBtn, &QPushButton::clicked, this, &ImageInteractionView::Start);
    connect(m_ui.ResetBtn, &QPushButton::clicked, this, &ImageInteractionView::Reset);
}

void ImageInteractionView::Reset()
{
       if (m_Interactor)
       {
           static_cast<ImageInteractor*>(m_Interactor.GetPointer())->Reset();
           RequestRenderWindowUpdate();
       }
}

void ImageInteractionView::Start(bool start)
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
             m_Interactor = ImageInteractor::New();
             m_Interactor->LoadStateMachine(configPath + "/mitk/Interactions/ImageNavigation.xml");
             m_Interactor->SetEventConfig(configPath + "/mitk/Interactions/ImageNavigationConfig.xml"); 

             itk::SimpleMemberCommand<ImageInteractionView>::Pointer imageChanged =
                 itk::SimpleMemberCommand<ImageInteractionView>::New();
             imageChanged->SetCallbackFunction(this, &ImageInteractionView::ImageMatrixChanged);
             m_Interactor->AddObserver(itk::ModifiedEvent(), imageChanged);

         }
         m_Interactor->SetDataNode(m_ui.DataSelector->GetSelectedNode());
         m_ui.DataSelector->GetSelectedNode()->SetDataInteractor(m_Interactor);
     }
     else
     {
         m_ui.DataSelector->GetSelectedNode()->SetDataInteractor(nullptr);
     }
}

void  ImageInteractionView::ImageMatrixChanged()
{
    ImageInteractor* interactor = static_cast<ImageInteractor*>(m_Interactor.GetPointer());
    
    QMatrix4x4 matrix = interactor->GetTransformMatrix();
    QString matrixStr = QString("%1 %2 %3 %4\n%5 %6 %7 %8\n%9 %10 %11 %12\n%13 %14 %15 %16")
        .arg(matrix(0, 0)).arg(matrix(0, 1)).arg(matrix(0, 2)).arg(matrix(0, 3))
        .arg(matrix(1, 0)).arg(matrix(1, 1)).arg(matrix(1, 2)).arg(matrix(1, 3))
        .arg(matrix(2, 0)).arg(matrix(2, 1)).arg(matrix(2, 2)).arg(matrix(2, 3))
        .arg(matrix(3, 0)).arg(matrix(3, 1)).arg(matrix(3, 2)).arg(matrix(3, 3));
    m_ui.TransformMatrix->setPlainText(matrixStr);

}
