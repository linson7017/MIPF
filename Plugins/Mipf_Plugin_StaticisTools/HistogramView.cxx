#include "HistogramView.h"
#include "iqf_main.h"
#include "Res/R.h"

//qwt
#include "qwt_plot_histogram.h"
#include "qwt_plot_canvas.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_panner.h"
#include "qwt_plot_magnifier.h"

//itk
#include "itkImageToHistogramFilter.h"
#include "itkImageRandomIteratorWithIndex.h"
#include "itkLineIterator.h"
#include "itkSampleToHistogramFilter.h"
#include "itkListSample.h"
#include "itkHistogram.h"


//mitk
#include "mitkImageCast.h"
#include "QmitkStdMultiWidget.h"
#include "mitkImageVtkMapper2D.h"
#include "mitkResliceMethodProperty.h"
#include "mitkAbstractTransformGeometry.h"
#include "vtkMitkThickSlicesFilter.h"
#include "mitkPlaneClipping.h"
#include "vtkMitkLevelWindowFilter.h"
#include "vtkImageExtractComponents.h"

//vtk
#include "vtkJPEGReader.h"
#include "vtkQImageToImageSource.h"
#include "vtkUnsignedCharArray.h"
#include "vtkPointData.h"
#include "mitkVtkResliceInterpolationProperty.h"

//common
#include "ITKImageTypeDef.h"
#include "VesselTools/IQF_VesselSegmentationTool.h"
#include "MitkImageUtils/IQF_MitkImageReslice.h"
#include "VTK_Helpers.h"



#include "CurveProjectionMapper2D.h"
#include "TransparentBackgroundMapper2D.h"

#include <QVector2D>

class Histogram : public QwtPlotHistogram
{
public:
    Histogram(const QString &, const QColor &);

    void setColor(const QColor &);
    void setValues(uint numValues, const double *);
};

Histogram::Histogram(const QString &title, const QColor &symbolColor) :
    QwtPlotHistogram(title)
{
    setStyle(QwtPlotHistogram::Columns);

    setColor(symbolColor);
}

void Histogram::setColor(const QColor &color)
{
    QColor c = color;
    c.setAlpha(180);
    setBrush(QBrush(c));
}

void Histogram::setValues(uint numValues, const double *values)
{
    QVector<QwtIntervalSample> samples(numValues);
    for (uint i = 0; i < numValues; i++)
    {
        QwtInterval interval(double(i), i + 1.0);
        interval.setBorderFlags(QwtInterval::ExcludeMaximum);

        samples[i] = QwtIntervalSample(values[i], interval);
    }

    setData(new QwtIntervalSeriesData(samples));
}


HistogramView::HistogramView() :MitkPluginView()
{
}

HistogramView::~HistogramView() 
{
}

void HistogramView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.DataSelector->SetDataStorage(GetDataStorage());
    m_ui.DataSelector->SetPredicate(CreatePredicate(Image));
    {
        QwtPlotCanvas *canvas = new QwtPlotCanvas();
        canvas->setPalette(Qt::gray);
        canvas->setBorderRadius(10);
        m_ui.HistogramPlot->setCanvas(canvas);
        m_ui.HistogramPlot->plotLayout()->setAlignCanvasToScales(true);
        m_ui.HistogramPlot->setAxisTitle(QwtPlot::yLeft, "Number of bins");
        m_ui.HistogramPlot->setAxisTitle(QwtPlot::xBottom, "Gray Value");

        //panning
        (void)new QwtPlotPanner(canvas);
        //zooming
        QwtPlotMagnifier *magnifier = new QwtPlotMagnifier(canvas);
        magnifier->setMouseButton(Qt::NoButton);
    }
    
    {
        QwtPlotCanvas *canvas = new QwtPlotCanvas();
        canvas->setPalette(Qt::gray);
        canvas->setBorderRadius(10);
        m_ui.ProfileHistogramPlot->setCanvas(canvas);
        m_ui.ProfileHistogramPlot->plotLayout()->setAlignCanvasToScales(true);
        m_ui.ProfileHistogramPlot->setAxisTitle(QwtPlot::yLeft, "Number of bins");
        m_ui.ProfileHistogramPlot->setAxisTitle(QwtPlot::xBottom, "Gray Value");

        //panning
        (void)new QwtPlotPanner(canvas);
        //zooming
        QwtPlotMagnifier *magnifier = new QwtPlotMagnifier(canvas);
        magnifier->setMouseButton(Qt::NoButton);
    }    

    m_histogram = new Histogram("Histogram", Qt::red);
    m_histogram->attach(m_ui.HistogramPlot);

    m_profileHistogram = new Histogram("Profile Histogram", Qt::red);
    m_profileHistogram->attach(m_ui.ProfileHistogramPlot);


    
    m_ui.ImageWidget->setScaledContents(true);
    m_qImage = new QImage;

    connect(m_ui.PlotBtn, SIGNAL(clicked()), this, SLOT(Plot()));
    connect(m_ui.StartPointBtn, SIGNAL(clicked()), this, SLOT(OnSelectStartPoint()));
    connect(m_ui.EndPointBtn, SIGNAL(clicked()), this, SLOT(OnSelectEndPoint()));
    connect(m_ui.ProfilePlotBtn, SIGNAL(clicked()), this, SLOT(ProfilePlot()));

    connect(&m_timer, SIGNAL(timeout()), this, SLOT(Refresh()));
}

int imageNum = 0;

void HistogramView::Refresh()
{
    QString fileName = QString("S:/Data/UR/%1.jpg").arg(imageNum);
    if (imageNum>61)
    {
        imageNum = 0;
    }
    else
    {
        imageNum++;
    }

    m_qImage->load(fileName);
    m_ui.ImageWidget->setFixedSize(m_qImage->width()/3, m_qImage->height()/3);
    m_qImage->convertToFormat(QImage::Format_RGBA8888);

    m_ui.ImageWidget->setPixmap(QPixmap::fromImage(*m_qImage));



    vtkSmartPointer<vtkQImageToImageSource> converter = vtkSmartPointer<vtkQImageToImageSource>::New();
    converter->SetQImage(m_qImage);
    converter->Update();

    vtkImageData* image = converter->GetOutput();
    

    m_image->Initialize(image);
    m_image->GetVtkImageData()->DeepCopy(image);
    

    RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);
   // m_pMitkRenderWindow->GetMitkStdMultiWidget()->ForceImmediateUpdate();
   // RequestRenderWindowUpdate(mitk::RenderingManager::REQUEST_UPDATE_2DWINDOWS);

}

bool HistogramView::GetReslicePlaneImage(mitk::DataNode* imageNode,const  mitk::PlaneGeometry* worldGeometry, mitk::BaseRenderer* renderer, mitk::Image* output)
{
    //mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    if (!imageNode || !output || !worldGeometry || !renderer)
    {
        return false;
    }

    //mitk::ExtractSliceFilter::Pointer  reslicer = mitk::ExtractSliceFilter::New();
    //reslicer->SetInput(mitkImage);
    //reslicer->SetWorldGeometry(planeGeometry);
    //reslicer->SetTimeStep(renderer->GetTimeStep(mitkImage));
    //reslicer->SetResliceTransformByGeometry(
    //    mitkImage->GetTimeGeometry()->GetGeometryForTimeStep(renderer->GetTimeStep(mitkImage)));
    //bool inPlaneResampleExtentByGeometry = false;
    //m_ui.DataSelector->GetSelectedNode()->GetBoolProperty("in plane resample extent by geometry", inPlaneResampleExtentByGeometry, renderer);
    //reslicer->SetInPlaneResampleExtentByGeometry(inPlaneResampleExtentByGeometry);
    //reslicer->SetInterpolationMode(mitk::ExtractSliceFilter::RESLICE_NEAREST);
    //reslicer->SetVtkOutputRequest(true);

    //reslicer->SetOutputDimensionality(2);
    //reslicer->SetOutputSpacingZDirection(1.0);
    //reslicer->SetOutputExtentZDirection(0, 0);
    //reslicer->Modified();
    //// start the pipeline with updating the largest possible, needed if the geometry of the input has changed
    //reslicer->UpdateLargestPossibleRegion();

    //output->Initialize(reslicer->GetVtkOutput());
    //output->GetGeometry()->SetIndexToWorldTransform((mitk::AffineTransform3D*)planeGeometry->GetIndexToWorldTransform());
    //output->SetVolume(reslicer->GetVtkOutput()->GetScalarPointer());
    mitk::Image *image = dynamic_cast<mitk::Image*>(imageNode->GetData());
    mitk::DataNode *datanode = imageNode;
    auto reslicedImage = vtkSmartPointer<vtkImageData>::New();
    auto levelWindowFilter = vtkSmartPointer<vtkMitkLevelWindowFilter>::New();
    if (nullptr == image || !image->IsInitialized())
    {
        return false;
    }
    // check if there is a valid worldGeometry
    if (nullptr == worldGeometry || !worldGeometry->IsValid() || !worldGeometry->HasReferenceGeometry())
    {
        return false;
    }

    image->Update();


    // set main input for ExtractSliceFilter
    mitk::ExtractSliceFilter::Pointer  reslicer = mitk::ExtractSliceFilter::New();
   reslicer->SetInput(image);
   reslicer->SetWorldGeometry(worldGeometry);
   reslicer->SetTimeStep(renderer->GetTimeStep(image));
   reslicer->SetResliceTransformByGeometry(
        image->GetTimeGeometry()->GetGeometryForTimeStep(renderer->GetTimeStep(image)));
    bool inPlaneResampleExtentByGeometry = false;
    datanode->GetBoolProperty("in plane resample extent by geometry", inPlaneResampleExtentByGeometry, renderer);
   reslicer->SetInPlaneResampleExtentByGeometry(inPlaneResampleExtentByGeometry);


    if ((image->GetDimension() >= 3) && (image->GetDimension(2) > 1))
    {
        mitk::VtkResliceInterpolationProperty *resliceInterpolationProperty;
        datanode->GetProperty(resliceInterpolationProperty, "reslice interpolation", renderer);

        int interpolationMode = VTK_RESLICE_NEAREST;
        if (resliceInterpolationProperty != NULL)
        {
            interpolationMode = resliceInterpolationProperty->GetInterpolation();
        }

        switch (interpolationMode)
        {
        case VTK_RESLICE_NEAREST:
           reslicer->SetInterpolationMode(mitk::ExtractSliceFilter::RESLICE_NEAREST);
            break;
        case VTK_RESLICE_LINEAR:
           reslicer->SetInterpolationMode(mitk::ExtractSliceFilter::RESLICE_LINEAR);
            break;
        case VTK_RESLICE_CUBIC:
           reslicer->SetInterpolationMode(mitk::ExtractSliceFilter::RESLICE_CUBIC);
            break;
        }
    }
    else
    {
       reslicer->SetInterpolationMode(mitk::ExtractSliceFilter::RESLICE_NEAREST);
    }

   reslicer->SetVtkOutputRequest(true);

    int thickSlicesMode = 0;
    int thickSlicesNum = 1;
    if (image->GetPixelType().GetNumberOfComponents() == 1) 
    {
        mitk::DataNode *dn = renderer->GetCurrentWorldPlaneGeometryNode();
        if (dn)
        {
            mitk::ResliceMethodProperty *resliceMethodEnumProperty = 0;

            if (dn->GetProperty(resliceMethodEnumProperty, "reslice.thickslices", renderer) && resliceMethodEnumProperty)
                thickSlicesMode = resliceMethodEnumProperty->GetValueAsId();

            mitk::IntProperty *intProperty = 0;
            if (dn->GetProperty(intProperty, "reslice.thickslices.num", renderer) && intProperty)
            {
                thickSlicesNum = intProperty->GetValue();
                if (thickSlicesNum < 1)
                    thickSlicesNum = 1;
            }
        }
        else
        {
            MITK_WARN << "no associated widget plane data tree node found";
        }
    }

    const mitk::PlaneGeometry *planeGeometry = dynamic_cast<const mitk::PlaneGeometry *>(worldGeometry);

    if (thickSlicesMode > 0)
    {
        double dataZSpacing = 1.0;

        mitk::Vector3D normInIndex, normal;

        const mitk::AbstractTransformGeometry *abstractGeometry =
            dynamic_cast<const mitk::AbstractTransformGeometry *>(worldGeometry);
        if (abstractGeometry != NULL)
            normal = abstractGeometry->GetPlane()->GetNormal();
        else
        {
            if (planeGeometry != NULL)
            {
                normal = planeGeometry->GetNormal();
            }
            else
                return false; // no fitting geometry set
        }
        normal.Normalize();

        image->GetTimeGeometry()->GetGeometryForTimeStep(renderer->GetTimeStep(image))->WorldToIndex(normal, normInIndex);

        dataZSpacing = 1.0 / normInIndex.GetNorm();

       reslicer->SetOutputDimensionality(3);
       reslicer->SetOutputSpacingZDirection(dataZSpacing);
       reslicer->SetOutputExtentZDirection(-thickSlicesNum, 0 + thickSlicesNum);

        // Do the reslicing. Modified() is called to make sure that the reslicer is
        // executed even though the input geometry information did not change; this
        // is necessary when the input /em data, but not the /em geometry changes.
       auto pTSFilter = vtkSmartPointer<vtkMitkThickSlicesFilter>::New();
       pTSFilter->SetThickSliceMode(thickSlicesMode - 1);
       pTSFilter->SetInputData(reslicer->GetVtkOutput());

        // vtkFilter=>mitkFilter=>vtkFilter update mechanism will fail without calling manually
       reslicer->Modified();
       reslicer->Update();

        pTSFilter->Modified();
        pTSFilter->Update();
        reslicedImage = pTSFilter->GetOutput();
    }
    else
    {
        // this is needed when thick mode was enable bevore. These variable have to be reset to default values
       reslicer->SetOutputDimensionality(2);
       reslicer->SetOutputSpacingZDirection(1.0);
       reslicer->SetOutputExtentZDirection(0, 0);

       reslicer->Modified();
        // start the pipeline with updating the largest possible, needed if the geometry of the input has changed
       reslicer->UpdateLargestPossibleRegion();
       reslicedImage =reslicer->GetVtkOutput();
    }

    // Bounds information for reslicing (only reuqired if reference geometry
    // is present)
    // this used for generating a vtkPLaneSource with the right size
    double sliceBounds[6];
    for (auto &sliceBound : sliceBounds)
    {
        sliceBound = 0.0;
    }
   reslicer->GetClippedPlaneBounds(sliceBounds);

    // get the spacing of the slice
   double* mmPerPixel = reslicer->GetOutputSpacing();

    // calculate minimum bounding rect of IMAGE in texture
    {
        double textureClippingBounds[6];
        for (auto &textureClippingBound : textureClippingBounds)
        {
            textureClippingBound = 0.0;
        }
        // Calculate the actual bounds of the transformed plane clipped by the
        // dataset bounding box; this is required for drawing the texture at the
        // correct position during 3D mapping.
        mitk::PlaneClipping::CalculateClippedPlaneBounds(image->GetGeometry(), planeGeometry, textureClippingBounds);

        textureClippingBounds[0] = static_cast<int>(textureClippingBounds[0] / mmPerPixel[0] + 0.5);
        textureClippingBounds[1] = static_cast<int>(textureClippingBounds[1] / mmPerPixel[0] + 0.5);
        textureClippingBounds[2] = static_cast<int>(textureClippingBounds[2] / mmPerPixel[1] + 0.5);
        textureClippingBounds[3] = static_cast<int>(textureClippingBounds[3] / mmPerPixel[1] + 0.5);

        // clipping bounds for cutting the image
        levelWindowFilter->SetClippingBounds(textureClippingBounds);
    }

    // get the number of scalar components to distinguish between different image types
    int numberOfComponents = reslicedImage->GetNumberOfScalarComponents();
    int extent[6];
    reslicedImage->GetExtent(extent);
    VTKHelpers::SaveVtkImageData(reslicedImage, "D:/temp/resliced.mha");
    // get the binary property
    //bool binary = false;
    //bool binaryOutline = false;
    //datanode->GetBoolProperty("binary", binary, renderer);
    //if (binary) // binary image
    //{
    //    datanode->GetBoolProperty("outline binary", binaryOutline, renderer);
    //    if (binaryOutline) // contour rendering
    //    {
    //        // get pixel type of vtk image
    //        itk::ImageIOBase::IOComponentType componentType = static_cast<itk::ImageIOBase::IOComponentType>(image->GetPixelType().GetComponentType());
    //        switch (componentType)
    //        {
    //        case itk::ImageIOBase::UCHAR:
    //            // generate contours/outlines
    //            localStorage->m_OutlinePolyData = CreateOutlinePolyData<unsigned char>(renderer);
    //            break;
    //        case itk::ImageIOBase::USHORT:
    //            // generate contours/outlines
    //            localStorage->m_OutlinePolyData = CreateOutlinePolyData<unsigned short>(renderer);
    //            break;
    //        default:
    //            binaryOutline = false;
    //            this->ApplyLookuptable(renderer);
    //            MITK_WARN << "Type of all binary images should be unsigned char or unsigned short. Outline does not work on other pixel types!";
    //        }
    //        if (binaryOutline) // binary outline is still true --> add outline
    //        {
    //            float binaryOutlineWidth = 1.0;
    //            if (datanode->GetFloatProperty("outline width", binaryOutlineWidth, renderer))
    //            {
    //                if (localStorage->m_Actors->GetNumberOfPaths() > 1)
    //                {
    //                    float binaryOutlineShadowWidth = 1.5;
    //                    datanode->GetFloatProperty("outline shadow width", binaryOutlineShadowWidth, renderer);
    //                    dynamic_cast<vtkActor *>(localStorage->m_Actors->GetParts()->GetItemAsObject(0))
    //                        ->GetProperty()
    //                        ->SetLineWidth(binaryOutlineWidth * binaryOutlineShadowWidth);
    //                }
    //                localStorage->m_Actor->GetProperty()->SetLineWidth(binaryOutlineWidth);
    //            }
    //        }
    //    }
    //    else // standard binary image
    //    {
    //        if (numberOfComponents != 1)
    //        {
    //            MITK_ERROR << "Rendering Error: Binary Images with more then 1 component are not supported!";
    //        }
    //    }
    //}


    //this->ApplyRenderingMode(renderer);


    //// do not use a VTK lookup table (we do that ourselves in m_LevelWindowFilter)
    //localStorage->m_Texture->MapColorScalarsThroughLookupTableOff();

    int displayedComponent = 0;
    if (datanode->GetIntProperty("Image.Displayed Component", displayedComponent, renderer) && numberOfComponents > 1)
    {
        auto vectorComponentExtractor = vtkSmartPointer<vtkImageExtractComponents>::New();
        vectorComponentExtractor->SetComponents(displayedComponent);
        vectorComponentExtractor->SetInputData(reslicedImage);

        levelWindowFilter->SetInputConnection(vectorComponentExtractor->GetOutputPort(0));
    }
    else
    {
        // connect the input with the levelwindow filter
        levelWindowFilter->SetInputData(reslicedImage);
    }
    output->Initialize(levelWindowFilter->GetOutput());
    output->GetGeometry()->SetIndexToWorldTransform((mitk::AffineTransform3D*)planeGeometry->GetIndexToWorldTransform());
    output->SetVolume(levelWindowFilter->GetOutput()->GetScalarPointer());
    return true;
}

void HistogramView::Plot()
{
   
   //cut image in cpu
    /************************************************************************/
    /* conver vtk rgb image to rgba image                                                                     */
    /************************************************************************/
    /*vtkJPEGReader* reader = vtkJPEGReader::New();
    reader->SetFileName("D:/temp/UR.jpg");
    reader->Update();
    vtkImageData* originImage = reader->GetOutput();
    vtkImageData* image = vtkImageData::New();
    image->ShallowCopy(reader->GetOutput());
    image->AllocateScalars(VTK_UNSIGNED_CHAR, 4);

    int dimens[3];
    image->GetDimensions(dimens);
    unsigned char* data = new unsigned char[4 * dimens[0]*dimens[1]];
    memset(data, 0, 4 * dimens[0] * dimens[1]);
    vtkUnsignedCharArray* array = vtkUnsignedCharArray::SafeDownCast(image->GetPointData()->GetScalars());
    array->SetVoidArray(data, 4 * dimens[0] * dimens[1], 0, vtkUnsignedCharArray::VTK_DATA_ARRAY_DELETE);

    double smallRadiusSquared = 258 * 258;
    double largeRadiusSquared = 882 * 882;
    QVector2D u(0, 1);
    QVector2D b(564 - 436, 1 + 225);
    double cosTheta = QVector2D::dotProduct(u.normalized(), b.normalized());
    for (int y = 0; y < dimens[1]; y++)
    {
        for (int x = 0; x < dimens[0]; x++)
        {
            unsigned char* opixel = static_cast<unsigned char*>(originImage->GetScalarPointer(x, y, 0));
            unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(x, y, 0));

            QVector2D p(x - 436, 884 - y);
            double distance = p.lengthSquared();
            if (distance< smallRadiusSquared ||
                distance> largeRadiusSquared ||
                QVector2D::dotProduct(u.normalized(), p.normalized()) < cosTheta)
            {
                pixel[3] = 0;
            }
            else
            {
                pixel[0] = opixel[0];
                pixel[1] = opixel[1];
                pixel[2] = opixel[2];
                pixel[3] = 255;
            }
        }
    }*/

    /************************************************************************/
    /*Cut image in CPU                                                                     */
    /************************************************************************/
   /* int numberOfComponent = image->GetNumberOfScalarComponents();

    int* dims = image->GetDimensions();

    double smallRadiusSquared = 258 * 258;
    double largeRadiusSquared = 882 * 882;
    QVector2D u(0, 1);
    QVector2D b(564 - 436, 1 + 225);
    double cosTheta = QVector2D::dotProduct(u.normalized(), b.normalized());

    for (int y = 0; y < dims[1]; y++)
    {
        for (int x = 0; x < dims[0]; x++)
        {
            unsigned char* pixel = static_cast<unsigned char*>(image->GetScalarPointer(x, y, 0));

            QVector2D p(x - 436, 884 - y);
            double distance = p.lengthSquared();
            if (distance< smallRadiusSquared ||
                distance> largeRadiusSquared ||
                QVector2D::dotProduct(u.normalized(), p.normalized()) < cosTheta)
            {
                pixel[3] = 0;
            }
        }
    }*/

    //reslice data

    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    const mitk::DataNode* planeGeometryNode = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow1()->GetRenderer()->GetCurrentWorldGeometry2DNode();
    const mitk::PlaneGeometry* planeGeometry = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow1()->GetRenderer()->GetCurrentWorldPlaneGeometry();
    mitk::PlaneGeometryData* planeGeometryNodeData = dynamic_cast<mitk::PlaneGeometryData*>(planeGeometryNode->GetData());

    //vtk reslice
    /*vtkSmartPointer<vtkImageReslice> reslicer = vtkSmartPointer<vtkImageReslice>::New();
    reslicer->SetInputData(mitkImage->GetVtkImageData());
    reslicer->SetOutputExtent(0, planeGeometryNodeData->GetGeometry()->GetExtent(0),
        0, planeGeometryNodeData->GetGeometry()->GetExtent(1),
        0, 0);
    reslicer->SetResliceAxes(planeGeometryNodeData->GetGeometry()->GetVtkMatrix());
    reslicer->Update();*/
    IQF_MitkImageReslice* pReslice = (IQF_MitkImageReslice*)GetInterfacePtr(QF_MitkImageUtils_ImageReslice);


    vtkImageData* vim = vtkImageData::New();
    
    mitk::BaseRenderer* renderer = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow1()->GetRenderer();
    pReslice->GetReslicePlaneImageWithLevelWindow(m_ui.DataSelector->GetSelectedNode(), planeGeometry,
        renderer, vim);
    //VTKHelpers::SaveVtkImageData(vim, "D:/temp/resliced.mha");

    
    mitkImage->Update();
    //mitk reslice
    mitk::Image::Pointer mIm = mitk::Image::New();
    mIm->Initialize(vim);
    mIm->SetVolume(vim->GetScalarPointer());

    mIm->GetGeometry()->SetIndexToWorldTransform(planeGeometryNode->GetData()->GetGeometry()->GetIndexToWorldTransform());
    mitk::Point3D origin = planeGeometry->GetOrigin();
    mitk::Vector3D spacing = mIm->GetGeometry()->GetSpacing();
    origin.SetElement(0, origin.GetElement(0) - spacing.GetElement(0) / 2);
    origin.SetElement(1, origin.GetElement(1) - spacing.GetElement(1) / 2);
    mIm->GetGeometry()->SetOrigin(origin);

    mitk::DataNode::Pointer mNode = mitk::DataNode::New();
    mNode->SetData(mIm);
    mNode->SetName("reslice image");

    /*mitk::TransparentBackgroundMapper2D::Pointer mapper3D = mitk::TransparentBackgroundMapper2D::New();
    mNode->SetMapper(mitk::BaseRenderer::Standard2D, mapper3D);
    mNode->SetStringProperty("2d mapper type", "shader rendering");*/

    GetDataStorage()->Add(mNode);

    //US
    m_image = mitk::Image::New();
    
    m_dataNode = mitk::DataNode::New();
    Refresh();
    m_image->GetGeometry()->SetIndexToWorldTransform(planeGeometryNode->GetData()->GetGeometry()->GetIndexToWorldTransform());
    /*m_image->GetGeometry()->SetExtentInMM(0,planeGeometryNode->GetData()->GetGeometry()->GetExtentInMM(0));
    m_image->GetGeometry()->SetExtentInMM(1, planeGeometryNode->GetData()->GetGeometry()->GetExtentInMM(1));
    m_image->GetGeometry()->SetSpacing(planeGeometryNode->GetData()->GetGeometry()->GetSpacing());*/
    m_image->GetGeometry()->SetOrigin(planeGeometryNode->GetData()->GetGeometry()->GetOrigin());


    m_dataNode->SetData(m_image);
    m_dataNode->SetName("UR");
    GetDataStorage()->Add(m_dataNode);
    /************************************************************************/
    /* Cut image in GPU                                                                     */
    /************************************************************************/
    mitk::TransparentBackgroundMapper2D::Pointer mapper = mitk::TransparentBackgroundMapper2D::New();
    mapper->SetDataNode(m_dataNode);
    //mapper->SetFusion(true);
   // mapper->SetFusionDataNode(m_ui.DataSelector->GetSelectedNode());
    mapper->SetFusionRenderer(renderer);
    mapper->SetResliceDataNode(mNode);
    m_dataNode->SetMapper(mitk::BaseRenderer::Standard2D, mapper);
    m_dataNode->SetMapper(mitk::BaseRenderer::Standard3D, mapper);
    m_dataNode->SetStringProperty("2d mapper type", "shader rendering");

    float color[3] = { 0.0,0.0,1.0 };
    mitk::Color bkcolor(color);
    //m_pMitkRenderWindow->GetMitkStdMultiWidget()->SetGradientBackgroundColorForRenderWindow(bkcolor, bkcolor,3);
    m_pMitkRenderWindow->GetMitkStdMultiWidget()->SetGradientBackgroundColors(bkcolor, bkcolor);
    m_timer.start(20);


    return;

    //mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    //if (!mitkImage)
    //{
    //    return;
    //}
    //Int3DImageType::Pointer itkImage;
    //mitk::CastToItkImage(mitkImage, itkImage);

    //Int3DImageType::RegionType::SizeType imageSize = itkImage->GetLargestPossibleRegion().GetSize();
    //

    //const unsigned int MeasurementVectorSize = 1; // Grayscale
    //const unsigned int binsPerDimension = 255;
    //typedef itk::Statistics::ImageToHistogramFilter< Int3DImageType > ImageToHistogramFilterType;
    //ImageToHistogramFilterType::HistogramType::MeasurementVectorType lowerBound(binsPerDimension);
    //lowerBound.Fill(mitkImage->GetScalarValueMin());
    //ImageToHistogramFilterType::HistogramType::MeasurementVectorType upperBound(binsPerDimension);
    //upperBound.Fill(mitkImage->GetScalarValueMax());
    //ImageToHistogramFilterType::HistogramType::SizeType size(MeasurementVectorSize);
    //size.Fill(binsPerDimension);

    //ImageToHistogramFilterType::Pointer imageToHistogramFilter = ImageToHistogramFilterType::New();
    //imageToHistogramFilter->SetInput(itkImage);
    //imageToHistogramFilter->SetHistogramBinMinimum(lowerBound);
    //imageToHistogramFilter->SetHistogramBinMaximum(upperBound);
    //imageToHistogramFilter->SetHistogramSize(size);
    //imageToHistogramFilter->Update();
    //ImageToHistogramFilterType::HistogramType* histogram = imageToHistogramFilter->GetOutput();

    //uint numValues = histogram->GetSize()[0];
    //uint totalNum = imageSize[0] * imageSize[1] * imageSize[2];
    //QVector<QwtIntervalSample> samples(numValues);
    //for (uint i = 0; i < numValues; i++)
    //{
    //    double value = histogram->GetMeasurementVector(i).GetElement(0);
    //    QwtInterval interval(value, value + 1.0);
    //    interval.setBorderFlags(QwtInterval::ExcludeMaximum);
    //    samples[i] = QwtIntervalSample((double)histogram->GetFrequency(i)/ totalNum, interval);
    //}
    //std::cout <<"Bin Max:"<< histogram->GetDimensionMaxs(0).at(0)<< std::endl;
    //m_histogram->setData(new QwtIntervalSeriesData(samples));
    //m_ui.HistogramPlot->replot();



    
}


void HistogramView::OnSelectStartPoint()
{
    m_startPoint = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetCrossPosition();
    QString pointStr = QString("%1,%2,%3").arg(m_startPoint.GetElement(0)).arg(m_startPoint.GetElement(1)).arg(m_startPoint.GetElement(2));
    m_ui.StartPointLE->setText(pointStr);
}
void HistogramView::OnSelectEndPoint()
{
    m_endPoint = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetCrossPosition();
    QString pointStr = QString("%1,%2,%3").arg(m_endPoint.GetElement(0)).arg(m_endPoint.GetElement(1)).arg(m_endPoint.GetElement(2));
    m_ui.EndPointLE->setText(pointStr);
}

void HistogramView::DrawLine()
{
    if (m_pLineNode.IsNull())
    {
        m_pLineNode = mitk::DataNode::New();
        m_pLineNode->SetColor(1.0, 1.0, 0.0);
        m_pLineNode->SetProperty("show contour", mitk::BoolProperty::New(true));
        m_pLineNode->SetProperty("show distances", mitk::BoolProperty::New(true));
        m_pLineNode->SetProperty("helper object", mitk::BoolProperty::New(true));
        GetDataStorage()->Add(m_pLineNode);
    }
    if (m_linePoints.IsNull())
    {
        m_linePoints = mitk::PointSet::New();
        m_pLineNode->SetData(m_linePoints);
    }

    m_linePoints->Clear();
    m_linePoints->InsertPoint(m_startPoint);
    m_linePoints->InsertPoint(m_endPoint);
    m_pLineNode->Modified();


    RequestRenderWindowUpdate();
    ProfilePlot();
}

void HistogramView::ProfilePlot()
{
    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_ui.DataSelector->GetSelectedNode()->GetData());
    if (!mitkImage)
    {
        return;
    }
    Int3DImageType::Pointer itkImage;
    mitk::CastToItkImage(mitkImage, itkImage);

    Int3DImageType::IndexType startIndex, endIndex;
    mitkImage->GetGeometry()->WorldToIndex(m_startPoint, startIndex);
    mitkImage->GetGeometry()->WorldToIndex(m_endPoint, endIndex);

    //samples
    typedef itk::Vector<Int3DImageType::PixelType, 1>                       MeasurementVectorType;
    typedef itk::Statistics::ListSample< MeasurementVectorType >    SampleType;
    typedef itk::Statistics::Histogram< float,
        itk::Statistics::DenseFrequencyContainer2 >                             HistogramType;


    //create samples along the line
    SampleType::Pointer sample = SampleType::New();
    itk::LineIterator<Int3DImageType> it1(itkImage, startIndex, endIndex);
    it1.GoToBegin();
    MeasurementVectorType mv;
    while (!it1.IsAtEnd())
    {
        mv[0] = it1.Get();
        sample->PushBack(mv); 
        ++it1;
    }

    //Get histogram
    typedef itk::Statistics::SampleToHistogramFilter<SampleType, HistogramType> SampleToHistogramFilterType;
    SampleToHistogramFilterType::Pointer sampleToHistogramFilter =
        SampleToHistogramFilterType::New();
    sampleToHistogramFilter->SetInput(sample);

    SampleToHistogramFilterType::HistogramSizeType histogramSize(1);
    histogramSize.Fill(255);
    sampleToHistogramFilter->SetHistogramSize(histogramSize);
    sampleToHistogramFilter->Update();

    const HistogramType* histogram = sampleToHistogramFilter->GetOutput();


    uint numValues = histogram->GetSize()[0];
    uint totalNum = sample->Size();
    QVector<QwtIntervalSample> samples(numValues);
    for (uint i = 0; i < numValues; i++)
    {
        double value = histogram->GetMeasurementVector(i).GetElement(0);
        QwtInterval interval(value, value + 1.0);
        interval.setBorderFlags(QwtInterval::ExcludeMaximum);
        samples[i] = QwtIntervalSample((double)histogram->GetFrequency(i) / totalNum, interval);
    }
    m_profileHistogram->setData(new QwtIntervalSeriesData(samples));
    m_ui.ProfileHistogramPlot->replot();
}