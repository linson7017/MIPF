#include "MaskImageView.h"
#include "iqf_main.h"
#include "Res/R.h"
#include "ITKImageTypeDef.h"

//Qt
#include <QInputDialog>

//qmitk
#include "QmitkDataStorageComboBox.h"

//mitk
#include "mitkImageCast.h"
#include "mitkLabelSetImage.h"
#include "mitkTransferFunction.h"
#include "mitkImageVtkMapper2D.h"
#include "QmitkStdMultiWidget.h"
#include "mitkExtractSliceFilter.h"

//itk
#include "itkMaskImageFilter.h"
#include "itkExtractImageFilter.h"



//vtk
#include <vtkJPEGReader.h>
#include <vtkImageReslice.h>
#include "MitkMain/IQF_MitkRenderWindow.h"

MaskImageView::MaskImageView() :MitkPluginView()
{
}


MaskImageView::~MaskImageView()
{
}

void MaskImageView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.ImageSelector->SetPredicate(CreatePredicate(Image));
    m_ui.ImageSelector->SetDataStorage(GetDataStorage());
    m_ui.MaskSelector->SetPredicate(CreatePredicate(Image));
    m_ui.MaskSelector->SetDataStorage(GetDataStorage());
    connect(m_ui.MaskBtn, SIGNAL(clicked()), this, SLOT(Mask()));
}


void MaskImageView::Mask()
{
    


    mitk::DataNode* node = m_ui.ImageSelector->GetSelectedNode();
    mitk::DataNode::Pointer planeNode = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow1()->GetRenderer()->GetCurrentWorldPlaneGeometryNode();
    mitk::ImageVtkMapper2D *imageMapper =
        dynamic_cast<mitk::ImageVtkMapper2D *>(node->GetMapper(1));
    if (!imageMapper && node->GetMapper(1))
    { //... check if it is the composite mapper
        std::string cname(node->GetMapper(1)->GetNameOfClass());
        if (!cname.compare("CompositeMapper")) // string.compare returns 0 if the two strings are equal.
        {
            // get the standard image mapper.
            // This is a special case in MITK and does only work for the CompositeMapper.
            imageMapper = dynamic_cast<mitk::ImageVtkMapper2D *>(node->GetMapper(3));
        }
    }
    if (imageMapper)
    {
       // imageMapper->GetLocalStorage()->m_Texture;
    }
   // return;

     
    QString imageName = QInputDialog::getText(NULL, "Input Result Name", "Image Name:");

    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.ImageSelector->GetSelectedNode()->GetData());

    const mitk::DataNode* planeGeometryNode = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow1()->GetRenderer()->GetCurrentWorldGeometry2DNode();
    const mitk::PlaneGeometry* planeGeometry = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow1()->GetRenderer()->GetCurrentWorldPlaneGeometry();
    mitk::PlaneGeometryData* planeGeometryNodeData = dynamic_cast<mitk::PlaneGeometryData*>(planeGeometryNode->GetData());

    MITK_INFO << "Extent: " << planeGeometryNodeData->GetPlaneGeometry()->GetExtent(0)<<","
        << planeGeometryNodeData->GetPlaneGeometry()->GetExtent(1) << ","
        << planeGeometryNodeData->GetPlaneGeometry()->GetExtent(2);

    //mitk::ExtractSliceFilter::Pointer  reslicer = mitk::ExtractSliceFilter::New();
    //reslicer->SetInput(mitkImage);
    //reslicer->SetWorldGeometry(planeGeometry);
    //reslicer->SetResliceTransformByGeometry(
    //    mitkImage->GetGeometry());
    //reslicer->SetInterpolationMode(mitk::ExtractSliceFilter::RESLICE_NEAREST);
    //reslicer->SetVtkOutputRequest(true);

    //reslicer->SetOutputDimensionality(2);
    //reslicer->SetOutputSpacingZDirection(1.0);
    //reslicer->SetOutputExtentZDirection(0, 0);

    //reslicer->Modified();
    //// start the pipeline with updating the largest possible, needed if the geometry of the input has changed
    //reslicer->UpdateLargestPossibleRegion();
    // vtkImageData* vim = reslicer->GetVtkOutput();

    vtkSmartPointer<vtkImageReslice> reslicer = vtkSmartPointer<vtkImageReslice>::New();
    reslicer->SetInputData(mitkImage->GetVtkImageData());
    reslicer->SetOutputExtent(0, planeGeometryNodeData->GetGeometry()->GetExtent(0),
        0, planeGeometryNodeData->GetGeometry()->GetExtent(1),
        0, planeGeometryNodeData->GetGeometry()->GetExtent(2));
    reslicer->SetResliceAxes(planeGeometryNodeData->GetGeometry()->GetVtkMatrix());
    reslicer->Update();
   
    vtkImageData* vim = reslicer->GetOutput();


    mitk::Image::Pointer mIm = mitk::Image::New();
    mIm->Initialize(vim);
    vtkMatrix4x4* tm = planeGeometryNodeData->GetGeometry()->GetVtkMatrix();
    vtkMatrix4x4* rm = vtkMatrix4x4::New();
    vtkMatrix4x4::Multiply4x4(tm, mIm->GetGeometry()->GetVtkMatrix(), rm);
    mIm->GetGeometry()->SetIndexToWorldTransformByVtkMatrix(rm);

    mIm->GetGeometry()->SetOrigin(planeGeometryNodeData->GetGeometry()->GetOrigin());

    mIm->SetVolume(vim->GetScalarPointer());
    mitk::DataNode::Pointer mNode = mitk::DataNode::New();
    mNode->SetData(mIm);
    
    mNode->SetName(imageName.toStdString());
    GetDataStorage()->Add(mNode);

    return;



    Float3DImageType::Pointer itkImage;
    mitk::CastToItkImage<Float3DImageType>(mitkImage, itkImage);

    mitk::Image* mitkMask = dynamic_cast<mitk::Image*>(m_ui.MaskSelector->GetSelectedNode()->GetData());
    UChar3DImageType::Pointer itkMask;
    mitk::CastToItkImage<UChar3DImageType>(mitkMask, itkMask);

    //check if the size is different
    bool useSmallestRegion = true;
    itk::ImageRegion<3> imageRegion = itkImage->GetLargestPossibleRegion();
    itk::ImageRegion<3> maskRegion = itkMask->GetLargestPossibleRegion();
    if (imageRegion!=maskRegion)
    {
        if (useSmallestRegion)
        {
            itk::ImageRegion<3> croppedRegion;
            if (imageRegion.Crop(maskRegion))
            {
                croppedRegion = imageRegion;
            }
            else if(maskRegion.Crop(imageRegion))
            {
                croppedRegion = maskRegion;
            }
            else
            {
                return;
            }
            typedef itk::ExtractImageFilter< Float3DImageType, Float3DImageType > ImageFilterType;
            ImageFilterType::Pointer imagefilter = ImageFilterType::New();
            imagefilter->SetExtractionRegion(croppedRegion);
            imagefilter->SetInput(itkImage);
            imagefilter->SetDirectionCollapseToIdentity(); // This is required.
            imagefilter->Update();
            itkImage->Graft(imagefilter->GetOutput());

            typedef itk::ExtractImageFilter< UChar3DImageType, UChar3DImageType > MaskeFilterType;
            MaskeFilterType::Pointer maskfilter = MaskeFilterType::New();
            maskfilter->SetExtractionRegion(croppedRegion);
            maskfilter->SetInput(itkMask);
            maskfilter->SetDirectionCollapseToIdentity(); // This is required.
            maskfilter->Update();
            itkMask->Graft(maskfilter->GetOutput());
        }
        else
        {
            std::cerr << "Image and mask have different size! " << std::endl;
            return;
        }
    }


    typedef itk::MaskImageFilter< Float3DImageType, UChar3DImageType > MaskFilterType;
    MaskFilterType::Pointer maskFilter = MaskFilterType::New();
    maskFilter->SetInput(itkImage);
    maskFilter->SetMaskImage(itkMask);
    maskFilter->Update();

    mitk::DataNode::Pointer maskedImageNode = mitk::DataNode::New();
    mitk::Image::Pointer maskedMitkImage = mitk::Image::New();
    mitk::CastToMitkImage(maskFilter->GetOutput(), maskedMitkImage);
    maskedImageNode->SetData(maskedMitkImage);
    maskedImageNode->SetColor(1, 1, 1);
    maskedImageNode->SetName(imageName.toStdString());

    m_pMitkDataManager->GetDataStorage()->Add(maskedImageNode);
}
