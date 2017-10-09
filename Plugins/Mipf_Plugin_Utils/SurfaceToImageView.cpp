#include "SurfaceToImageView.h"


#include "mitkSurfaceToImageFilter.h"

#include "VTK_Helpers.h"

#include <vtkImageData.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkDataArray.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkImageStencil.h>

SurfaceToImageView::SurfaceToImageView()
{
}


SurfaceToImageView::~SurfaceToImageView()
{
}

void SurfaceToImageView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Surface>::New());

    m_ui.ReferenceImageSelector->SetDataStorage(GetDataStorage());
    m_ui.ReferenceImageSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Image>::New());

    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &SurfaceToImageView::Apply);
}

WndHandle SurfaceToImageView::GetPluginHandle()
{
    return this;
}

void SurfaceToImageView::Apply()
{
    if (!m_ui.ReferenceImageSelector->GetSelectedNode() || !m_ui.DataSelector->GetSelectedNode())
    {
        return;
    }

    mitk::Image* refImage = dynamic_cast<mitk::Image*>(m_ui.ReferenceImageSelector->GetSelectedNode()->GetData());
    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_ui.DataSelector->GetSelectedNode()->GetData());

    if (!refImage||!surface)
    {
        return;
    }
    auto polydata = surface->GetVtkPolyData();
    double bounds[6];
    polydata->GetBounds(bounds);

    double spacing[3] = { 2.0,2.0,2.0 };
   
    int dim[3];
    for (int i = 0; i < 3; i++)
    {
        dim[i] = static_cast<int>(ceil((bounds[i * 2 + 1] - bounds[i * 2]) / spacing[i]));
    }
    int extent[6] = { 0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1 };

    double origin[3];
    origin[0] = bounds[0] + spacing[0] / 2;
    origin[1] = bounds[2] + spacing[1] / 2;
    origin[2] = bounds[4] + spacing[2] / 2;

    vtkSmartPointer<vtkPolyDataToImageStencil> pol2stenc =
        vtkSmartPointer<vtkPolyDataToImageStencil>::New();
    pol2stenc->SetInputData(surface->GetVtkPolyData());
    pol2stenc->SetOutputOrigin(origin);
    pol2stenc->SetOutputSpacing(spacing);
    pol2stenc->SetOutputWholeExtent(extent);
    pol2stenc->Update();

    // cut the corresponding white image and set the background:
    vtkSmartPointer<vtkImageStencil> imgstenc =
        vtkSmartPointer<vtkImageStencil>::New();
    imgstenc->SetInputData(CreateWhiteImage(spacing,origin,extent,dim));
    imgstenc->SetStencilData(pol2stenc->GetOutput());
    imgstenc->ReverseStencilOff();
    imgstenc->SetBackgroundValue(0);
    imgstenc->Update();

    VTKHelpers::SaveVtkImageData(imgstenc->GetOutput(), "D:/temp/stenc.mha");

    mitk::SlicedGeometry3D::Pointer geometry = mitk::SlicedGeometry3D::New();
    geometry->SetOrigin(origin);
    geometry->SetSpacing(spacing);
    geometry->SetImageGeometry(true);
    geometry->SetFloatBounds(bounds);

    ImportVTKImage(imgstenc->GetOutput(), "result", m_ui.DataSelector->GetSelectedNode(), geometry);
}

vtkSmartPointer<vtkImageData>  SurfaceToImageView::CreateWhiteImage(double* spacing, double* origin, int* extent, int* dim)
{
    
    unsigned char inval = 255;
    unsigned char outval = 0;

    vtkSmartPointer<vtkImageData> whiteImage =
        vtkSmartPointer<vtkImageData>::New();
    whiteImage->SetSpacing(spacing);
    whiteImage->SetOrigin(origin);
    whiteImage->SetDimensions(dim);
    whiteImage->SetExtent(extent);
    whiteImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

    
    vtkIdType count = whiteImage->GetNumberOfPoints();
    for (vtkIdType i = 0; i < count; ++i)
    {
        whiteImage->GetPointData()->GetScalars()->SetTuple1(i, inval);
    }
    //VTKHelpers::SaveVtkImageData(whiteImage.Get(), "D:/temp/white.mha");

    return whiteImage;
}
