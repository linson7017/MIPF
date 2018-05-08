#include "TestView.h" 
#include "iqf_main.h"  

#include "mitkImageCast.h"

#include "ITKImageTypeDef.h"

#include "itkThresholdImageFilter.h"

#include "Test/ITest.h"
  
TestView::TestView() :MitkPluginView() 
{
}
 
TestView::~TestView() 
{

}
 
void TestView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    
    m_ui.DataSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Image>::New());

    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &TestView::Apply);
   

    if (HasAttribute("test"))
    {
        GetAttribute("test");
    }
    
} 
 
WndHandle TestView::GetPluginHandle() 
{
    return this; 
}

void TestView::Apply()
{
    ITest* pTest = (ITest*)GetInterfacePtr(QF_INTERFACE_TEST);
    if (pTest)
    {
        MITK_INFO << pTest->Add(5, 7);
    }
    return;
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());

    Int3DImageType::Pointer itkImage;
    mitk::CastToItkImage(mitkImage, itkImage);

    typedef itk::ThresholdImageFilter<Int3DImageType> FilterType;
    FilterType::Pointer fiter = FilterType::New();
    fiter->SetInput(itkImage);
    fiter->SetUpper(2000);
    fiter->SetLower(500);
    fiter->Update();

    mitk::DataNode* node = ImportITKImage(fiter->GetOutput(), "result");
    node->SetColor(1.0, 1.0, 0.0);
    node->SetOpacity(0.8);
}