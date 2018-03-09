#include "GraphcutSegmentationViewUi.h"

#include "iqf_main.h"

#include <QtWidgets>
#include <QtConcurrent>

#include "mitkSurface.h"
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateOr.h>
#include <mitkNodePredicateProperty.h>
#include <mitkLabelSetImage.h>
#include <mitkContourModel.h>
#include <mitkImageCast.h>
#include <mitkContourModelSet.h>
#include <mitkToolManager.h>
#include <mitkToolManagerProvider.h>
#include <mitkSurfaceBasedInterpolationController.h>
#include <mitkMesh.h>

#include <mitkImageToSurfaceFilter.h>
#include <mitkImagePixelAccessor.h>
#include <QmitkDataStorageComboBox.h>

#include <mitkImageReadAccessor.h>

#include <mitkVtkResliceInterpolationProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkLevelWindowProperty.h>

#include <QVTKWidget.h>
#include <QmitkRenderWindow.h>
#include <QmitkStdMultiWidget.h>


//itk
#include "itkGaussianInterpolateImageFunction.h"

#include <vtkPolyData.h>
#include <vtkImageWeightedSum.h>
#include <vtkImageDataToPointSet.h>
#include <vtkXMLStructuredGridWriter.h>
#include <vtkImageResample.h>

#include "VTK_Helpers.h"
#include "ITKVTK_Helpers.h"
#include "ITK_Helpers.h"
#include "vtkExtractVOI.h"


#include "mitkPixelType.h"
#include "mitkOrganTypeProperty.h"

#include "mitkSurfaceToImageFilter.h"

#include "MitkSegmentation/IQF_MitkSegmentationTool.h"
#include "MitkSegmentation/IQF_MitkSurfaceTool.h"
#include "MitkStd/IQF_MitkPointList.h"
#include "MitkImageUtils/IQF_MitkImageCropper.h"
#include "MitkMain/IQF_MitkDisplayOption.h"

#include "Segmentation/IQF_SegmentationMethodFactory.h"

#include "ITK_Helpers.h"

void SegmentationOptionUi::SetOrgan(const QString& szOrganType)
{
    Organ = szOrganType;
    if (Organ.compare("Liver", Qt::CaseInsensitive) == 0)
    {
        Lambda = 0.08;
        Color[0] = 0.667;
        Color[1] = 0.0;
        Color[2] = 0.0;
        Level = 30.0;
        Window = 400.0;
    }
    else if (Organ.compare("Kidney", Qt::CaseInsensitive) == 0)
    {
        Lambda = 0.2;
        Color[0] = 1.0;
        Color[1] = 0.667;
        Color[2] = 0.0;
        Level = 40;
        Window = 300;
    }
    else if (Organ.compare("Spleen", Qt::CaseInsensitive) == 0)
    {
        Lambda = 0.1;
        Color[0] = 0.0;
        Color[1] = 0.33;
        Color[2] = 1.0;
        Level = 40;
        Window = 50;
    }
    else if (Organ.compare("Gallbladder", Qt::CaseInsensitive) == 0)
    {
        Lambda = 0.08;
        Color[0] = 0.0;
        Color[1] = 0.33;
        Color[2] = 0.0;
        Level = 30;
        Window = 400;
    }
    else if (Organ.compare("Pancreas", Qt::CaseInsensitive) == 0)
    {
        Lambda = 0.03;
        Color[0] = 0.667;
        Color[1] = 0.33;
        Color[2] = 1.0;
        Level = 40;
        Window = 400;
    }
    else
    {
        Lambda = 0.08;
        Color[0] = 1.0;
        Color[1] = 0.0;
        Color[2] = 0.0;
        Level = 30.0;
        Window = 400.0;
    }
}

GraphcutSegmentationViewUi::GraphcutSegmentationViewUi() :MitkPluginView(),
m_bInited(false),
m_tool(NULL),
m_bPaintForeground(true),
m_sourceSinkNode(NULL)
, m_SurfaceInterpolator(mitk::SurfaceInterpolationController::GetInstance()),
m_bSampleRate(-1)
{
    m_segOption.SetOrgan("Liver");
    m_currentResultName = "";

    m_roi[0] = 0;
    m_roi[1] = 0;
    m_roi[2] = 0;
    m_roi[3] = 0;
    m_roi[4] = 0;
    m_roi[5] = 0;

    itk::ReceptorMemberCommand<GraphcutSegmentationViewUi>::Pointer command2 = itk::ReceptorMemberCommand<GraphcutSegmentationViewUi>::New();
    command2->SetCallbackFunction(this, &GraphcutSegmentationViewUi::OnSurfaceInterpolationInfoChanged);
}


void GraphcutSegmentationViewUi::InitInterpolator(mitk::DataNode* imageNode)
{
    if (!m_InterpolatedSinkSurfaceNode)
    {
        m_InterpolatedSinkSurfaceNode = mitk::DataNode::New();
        m_InterpolatedSinkSurfaceNode->SetColor(0, 0, 1);
        //m_InterpolatedSinkSurfaceNode->SetProperty("helper object", mitk::BoolProperty::New(true));
    }

    if (!m_InterpolatedSourceSurfaceNode)
    {
        m_InterpolatedSourceSurfaceNode = mitk::DataNode::New();
        m_InterpolatedSourceSurfaceNode->SetColor(0, 1, 0);
        //m_InterpolatedSourceSurfaceNode->SetProperty("helper object", mitk::BoolProperty::New(true));
    }


    mitk::Vector3D spacing = imageNode->GetData()->GetGeometry()->GetSpacing();
    double minSpacing(100);
    double maxSpacing(0);
    for (int i = 0; i < 3; i++)
    {
        if (spacing[i] < minSpacing)
        {
            minSpacing = spacing[i];
        }
        else if (spacing[i] > maxSpacing)
        {
            maxSpacing = spacing[i];
        }
    }

    m_SurfaceInterpolator->SetMaxSpacing(maxSpacing);
    m_SurfaceInterpolator->SetMinSpacing(minSpacing);
    m_SurfaceInterpolator->SetDistanceImageVolume(50000);

    mitk::Image *segmentationImage = dynamic_cast<mitk::Image *>(imageNode->GetData());
    m_SurfaceInterpolator->SetCurrentInterpolationSession(segmentationImage);

}


void GraphcutSegmentationViewUi::OnSurfaceInterpolationInfoChanged(const itk::EventObject &)
{
    if (1)
    {
        if (m_Watcher.isRunning())
            m_Watcher.waitForFinished();
        m_Future = QtConcurrent::run(this, &GraphcutSegmentationViewUi::Run3DInterpolation);
        m_Watcher.setFuture(m_Future);
    }

}

void GraphcutSegmentationViewUi::OnSurfaceInterpolationFinished()
{
    mitk::Surface::Pointer interpolatedSurface = m_SurfaceInterpolator->GetInterpolationResult();
    if (interpolatedSurface.IsNotNull() && m_sourceSinkNode)
    {
        m_InterpolatedSinkSurfaceNode->SetData(interpolatedSurface);

        m_InterpolatedSinkSurfaceNode->SetVisibility(true);

        if (!GetDataStorage()->Exists(m_InterpolatedSinkSurfaceNode))
        {
            GetDataStorage()->Add(m_InterpolatedSinkSurfaceNode);
        }
    }
    else if (interpolatedSurface.IsNull())
    {
        if (GetDataStorage()->Exists(m_InterpolatedSinkSurfaceNode))
        {
            m_InterpolatedSinkSurfaceNode->SetVisibility(false);
        }
    }


}

void GraphcutSegmentationViewUi::EstimationSampleRate()
{
    mitk::Image* image = dynamic_cast<mitk::Image*>(m_refImageNode->GetData());
    unsigned int* dimens = image->GetDimensions();
    unsigned long imageSize = dimens[0] * dimens[1] * dimens[2];
    std::cout << "Image Size:" << imageSize << std::endl;
    m_bSampleRate = pow((double)imageSize / 10000000, 1.0 / 3.0);
    if (m_bSampleRate<1)
    {
        m_bSampleRate = 1;
    }
    std::cout << "Sample Rate:" << m_bSampleRate << std::endl;
}

void GraphcutSegmentationViewUi::Run3DInterpolation()
{
    m_SurfaceInterpolator->Interpolate();
}

void GraphcutSegmentationViewUi::GeneratedInterpolatedSourceAndSink()
{
    //source 
    mitk::Image *sourceImage = dynamic_cast<mitk::Image *>(m_sourceSinkNode->GetData());
    m_SurfaceInterpolator->SetCurrentInterpolationSession(sourceImage);
    m_SurfaceInterpolator->Interpolate();
    mitk::Surface::Pointer interpolatedSurface = m_SurfaceInterpolator->GetInterpolationResult();
    if (interpolatedSurface)
    {
        m_InterpolatedSourceSurfaceNode->SetData(interpolatedSurface);
        m_InterpolatedSourceSurfaceNode->SetName("Source Surface");
        GetDataStorage()->Add(m_InterpolatedSourceSurfaceNode);
        ConvertSurfaceToImage(interpolatedSurface, m_originMitkImage, static_cast<mitk::Image*>(m_sourceSinkNode->GetData()));
    }

}

void GraphcutSegmentationViewUi::ConvertSurfaceToImage(mitk::Surface* surface, mitk::Image* referenceImage, mitk::Image* output)
{
    mitk::SurfaceToImageFilter::Pointer s2iFilter = mitk::SurfaceToImageFilter::New();
    //输出为二值图像
    s2iFilter->MakeOutputBinaryOn();
    if (output->GetPixelType().GetComponentType() == itk::ImageIOBase::USHORT)
        s2iFilter->SetUShortBinaryPixelType(true);
    //输入mitk::Surface数据
    s2iFilter->SetInput(surface);

    //重要，设置面数据图像对应的mitk::Image数据
    s2iFilter->SetImage(referenceImage);
    s2iFilter->Update();

    mitk::Image::Pointer newSeg = s2iFilter->GetOutput();

    mitk::ImageReadAccessor readAccess(newSeg, newSeg->GetVolumeData());
    const void *cPointer = readAccess.GetData();

    if (output && cPointer)
    {
        output->SetVolume(cPointer);
    }
    else
    {
        return;
    }
}

void GraphcutSegmentationViewUi::Resmaple()
{
    Float3DImageType::Pointer itkImage = Float3DImageType::New();
    mitk::CastToItkImage<Float3DImageType>(dynamic_cast<mitk::Image *>(m_refImageNode->GetData()), itkImage);
    Float3DImageType::Pointer resampleImage = Float3DImageType::New();
    ITKHelpers::Resample(itkImage.GetPointer(), resampleImage.GetPointer(), m_ui.ResampleLE->text().toInt());

    //  ITKHelpers::SaveImage(resampleImage.GetPointer(), "D:/temp/resample.mhd");
    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitk::CastToMitkImage<Float3DImageType>(resampleImage, mitkImage);
    mitk::DataNode::Pointer resampleNode = mitk::DataNode::New();
    std::string name = "Resample-";
    name.append(m_ui.ResampleLE->text().toStdString());
    resampleNode->SetData(mitkImage);
    resampleNode->SetName(name);
    resampleNode->Update();
    GetDataStorage()->Add(resampleNode);
}


void GraphcutSegmentationViewUi::CropImage()
{
    IQF_MitkImageCropper* pCropper = (IQF_MitkImageCropper*)m_pMain->GetInterfacePtr(QF_MitkImageUtils_ImageCropper);
    mitk::DataNode::Pointer cropNode = mitk::DataNode::New();
    pCropper->CreateBoundingBoxNode(m_refImageNode, cropNode, "crop");
}

void GraphcutSegmentationViewUi::ExtractROI()
{
    CropImage();
    return;

    mitk::Image* image = dynamic_cast<mitk::Image*>(m_sourceSinkNode->GetData());
    UShort3DImageType::Pointer itkImage;
    mitk::CastToItkImage(image, itkImage);

    typedef itk::BinaryThresholdImageFilter<UShort3DImageType, UShort3DImageType>  btImageFilterType;
    btImageFilterType::Pointer btFilter = btImageFilterType::New();
    btFilter->SetInput(itkImage);
    btFilter->SetLowerThreshold(1);
    btFilter->SetUpperThreshold(1);
    btFilter->SetInsideValue(1);
    btFilter->SetOutsideValue(0);
    btFilter->Update();

    typedef itk::BinaryImageToShapeLabelMapFilter<UShort3DImageType> BinaryImageToShapeLabelMapFilterType;
    BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
    binaryImageToShapeLabelMapFilter->SetInputForegroundValue(1);
    binaryImageToShapeLabelMapFilter->SetInput(btFilter->GetOutput());
    binaryImageToShapeLabelMapFilter->Update();


    std::cout << "Label Object:" << binaryImageToShapeLabelMapFilter->GetOutput()->GetNumberOfLabelObjects() << std::endl;
    for (unsigned int i = 0; i < binaryImageToShapeLabelMapFilter->GetOutput()->GetNumberOfLabelObjects(); i++)
    {
        BinaryImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType* labelObject = binaryImageToShapeLabelMapFilter->GetOutput()->GetNthLabelObject(i);
        std::cout << "Object " << i << " has bounding box " << labelObject->GetBoundingBox() << std::endl;
        std::cout << "Object " << i << "Center " << labelObject->GetCentroid().GetElement(0) << ","
            << labelObject->GetCentroid().GetElement(1) << ","
            << labelObject->GetCentroid().GetElement(2) << std::endl;
    }

    //m_pCropObject = mitk::GeometryData::New();
    //m_pCropObjectNode = mitk::GeometryData::New();

    IQF_PointListFactory* pFactory = (IQF_PointListFactory*)m_pMain->GetInterfacePtr(QF_MitkStd_PointListFactory);
    IQF_MitkPointList* pList = pFactory->CreatePointList();
    pList->Initialize();
    mitk::DataNode::Pointer pointsNode = mitk::DataNode::New();
    pList->CreateNewPointSetNode(pointsNode);
    itk::Point<double, 3> center = binaryImageToShapeLabelMapFilter->GetOutput()->GetNthLabelObject(0)->GetCentroid();
    double width = 100;
    double height = 100;
    double depth = 100;
    int temp = 1;
    for (int i = -1; i<2; i += 2)
    {
        for (int j = -1; j < 2; j += 2)
        {
            for (int k = -1; k < 2; k += 2)
            {
                itk::Point<double, 3> p;
                std::cout << "Point " << temp << ":" << center.GetElement(0) + i*width << ","
                    << center.GetElement(1) + j*height << ","
                    << center.GetElement(2) + k*depth << std::endl;
                temp++;
                pList->InsertPoint(center.GetElement(0) + i*width,
                    center.GetElement(1) + j*height,
                    center.GetElement(2) + k*depth);
            }
        }
    }

}


void GraphcutSegmentationViewUi::Reset()
{
    m_sources.clear();
    m_sinks.clear();

    GetDataStorage()->Remove(GetDataStorage()->GetNamedNode("source"));
    GetDataStorage()->Remove(GetDataStorage()->GetNamedNode("sink"));
    GetDataStorage()->Remove(GetDataStorage()->GetNamedNode("Result"));
    m_sourceSinkNode = NULL;
    InitSourceAndSinkNodes();

}

void GraphcutSegmentationViewUi::Init()
{
    if (!m_refImageNode)
    {
        return;
    }
    m_currentResultName = QInputDialog::getText(NULL, "Input Result Name", "Image Name:");
    if (m_currentResultName.isEmpty())
    {
        return;
    }


    //change level window
    IQF_MitkDisplayOption* pDisplayOption = (IQF_MitkDisplayOption*)m_pMain->GetInterfacePtr(QF_MitkMain_DisplayOption);
    pDisplayOption->SetLevelWindow(m_refImageNode, m_segOption.Level, m_segOption.Window);

    m_originMitkImage = dynamic_cast<mitk::Image*>(m_refImageNode->GetData());
    mitk::BaseGeometry* originImageGeometry = m_originMitkImage->GetGeometry();
    mitk::Point3D originOrigin = m_originMitkImage->GetGeometry()->GetOrigin();
    if (!m_originMitkImage)
    {
        return;
    }
    vtkImageData* originImage = m_originMitkImage->GetVtkImageData();
    double range[2];
    originImage->GetScalarRange(range);

    m_pMitkRenderWindow->Reinit(m_refImageNode);
    m_graphcut->Init();
    InitSourceAndSinkNodes();
    //InitInterpolator(m_sourceImageNode.GetPointer());
    InitTool();

    m_bInited = true;
}

void GraphcutSegmentationViewUi::InitSourceAndSinkNodes()
{
    GetDataStorage()->Remove(m_sourceSinkNode);
    m_sourceSinkNode = mitk::DataNode::New();
    m_pSegTool->CreateLabelSetImageNode(m_refImageNode, m_sourceSinkNode, "seed");
    mitk::LabelSetImage* labelImage = dynamic_cast<mitk::LabelSetImage*>(m_sourceSinkNode->GetData());
    if (labelImage)
    {
        labelImage->GetActiveLabelSet()->RemoveLabel(1);
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
    GetDataStorage()->Add(m_sourceSinkNode);
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    toolManager->SetWorkingData(m_sourceSinkNode);
}

void GraphcutSegmentationViewUi::InitTool()
{
    if (m_tool)
    {
        return;
    }
    m_pSegTool->Initialize();

    mitk::Image* patientImage = dynamic_cast<mitk::Image*>(m_refImageNode->GetData());

    m_pSegTool->SetWorkingData(m_sourceSinkNode);
    m_pSegTool->SetReferenceData(m_refImageNode);

    // load interaction events
    ChangeTool("Paint");
    m_tool->SetSize(5);
}


void GraphcutSegmentationViewUi::ChangeTool(const QString& toolName)
{
    if (!m_bInited)
    {
        return;
    }
    m_tool = dynamic_cast<mitk::PaintbrushTool *>(m_pSegTool->ChangeTool(toolName.toStdString().c_str()));
    if (m_tool)
    {
        m_tool->SetEnable3DInterpolation(false);
        m_tool->SetSize(m_ui.PenSizeSlider->value());
        //m_tool->SetFeedbackContourVisible(false);
    }
    
}

void GraphcutSegmentationViewUi::SwitchToForeground()
{
    if (!m_sourceSinkNode)
    {
        return;
    }
    mitk::LabelSetImage* labelImage = dynamic_cast<mitk::LabelSetImage*>(m_sourceSinkNode->GetData());
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

void GraphcutSegmentationViewUi::SwitchToBackground()
{
    if (!m_sourceSinkNode)
    {
        return;
    }
    mitk::LabelSetImage* labelImage = dynamic_cast<mitk::LabelSetImage*>(m_sourceSinkNode->GetData());
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


void GraphcutSegmentationViewUi::RefreshSourceAndSink()
{
    if (!m_sourceSinkNode)
    {
        return;
    }
    m_sources.clear();
    m_sinks.clear();

    mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(m_sourceSinkNode->GetData());
    UChar3DImageType::Pointer itkImage = UChar3DImageType::New();
    mitk::CastToItkImage<UChar3DImageType>(mitkImage, itkImage);
    if (mitkImage)
    {
        if (m_ui.DownSample->isChecked())
        {
            UChar3DImageType::Pointer itkResampleImage = UChar3DImageType::New();
            // ITKHelpers::DilateImage(itkImage.GetPointer(), itkResampleImage.GetPointer(), 1);
            //  ITKHelpers::DilateImage(itkImage.GetPointer(), itkResampleImage.GetPointer(), 1, 2);
            //  ITKHelpers::SaveImage(itkResampleImage.GetPointer(), "D:/temp/dilateImage.mha");
            ITKHelpers::ResampleLabelImage<UChar3DImageType, UChar3DImageType, itk::NearestNeighborInterpolateImageFunction>(
                itkImage.GetPointer(), itkResampleImage.GetPointer(), m_ui.ResampleLE->text().toDouble());
         //   ITKHelpers::SaveImage(itkResampleImage.GetPointer(), "D:/temp/resampleseed.mha");
            //  mitk::Image::Pointer bridgeImage = mitk::Image::New();
            // mitk::CastToMitkImage<UChar3DImageType>(itkResampleImage, bridgeImage);


            /**************Points to voxel*******************/
            //std::vector< itk::Point<double, 3> > spoints;
            ////get sources
            //ITKVTKHelpers::GetNonzeroPoints(mitkImage->GetVtkImageData(), spoints, 1);
            //ITKVTKHelpers::PointsToPixels(bridgeImage->GetVtkImageData(), spoints, m_sources);
            ////get sinks
            //std::vector< itk::Point<double, 3> > kpoints;
            //ITKVTKHelpers::GetNonzeroPoints(mitkImage->GetVtkImageData(), kpoints, 2);
            //ITKVTKHelpers::PointsToPixels(bridgeImage->GetVtkImageData(), kpoints, m_sinks);

            /**************DownSample*******************/
            ITKHelpers::GetITKImageNonzeroPixels(itkResampleImage.GetPointer(), m_sources, 1);
            ITKHelpers::GetITKImageNonzeroPixels(itkResampleImage.GetPointer(), m_sinks, 2);

            /*mitk::Mesh::Pointer mesh = mitk::Mesh::New();
            for (int i = 0; i < spoints.size(); i++)
            {
            mitk::Point3D point;
            point.SetElement(0, spoints.at(i).GetElement(0));
            point.SetElement(1, spoints.at(i).GetElement(1));
            point.SetElement(2, spoints.at(i).GetElement(2));
            mesh->InsertPoint(point);
            }
            for (int i = 0; i < kpoints.size(); i++)
            {
            mitk::Point3D point;
            point.SetElement(0, kpoints.at(i).GetElement(0));
            point.SetElement(1, kpoints.at(i).GetElement(1));
            point.SetElement(2, kpoints.at(i).GetElement(2));
            mesh->InsertPoint(point);
            }*/

            /*TKHelpers::SetITKImagePixel<UChar3DImageType>(itkResampleImage, m_sources, 1);
            ITKHelpers::SetITKImagePixel<UChar3DImageType>(itkResampleImage, m_sinks, 2);*/
            /*mitk::CastToMitkImage<UChar3DImageType>(itkResampleImage, bridgeImage);
            GetDataStorage()->Remove(GetDataStorage()->GetNamedNode("resampled seed"));
            mitk::DataNode::Pointer resampledSeedImageNode = mitk::DataNode::New();
            resampledSeedImageNode->SetData(bridgeImage);
            resampledSeedImageNode->SetName("resampled seed");
            resampledSeedImageNode->SetOpacity(0.5);
            GetDataStorage()->Add(resampledSeedImageNode);
            RequestRenderWindowUpdate();*/
        }
        else
        {
            ITKHelpers::GetITKImageNonzeroPixels<UChar3DImageType>(itkImage.GetPointer(), m_sources, 1);
            ITKHelpers::GetITKImageNonzeroPixels<UChar3DImageType>(itkImage.GetPointer(), m_sinks, 2);
        }
    }



}

void GraphcutSegmentationViewUi::RefreshROI()
{
    mitk::Image* mitkSourceSinkImage = dynamic_cast<mitk::Image*>(m_sourceSinkNode->GetData());
    if (mitkSourceSinkImage)
    {
        VTKHelpers::FindVTKImageROI(mitkSourceSinkImage->GetVtkImageData(), m_roi);
        std::cout << "ROI:" << m_roi[0] << "," << m_roi[1] << "," << m_roi[2] << "," << m_roi[3] << "," << m_roi[4] << "," << m_roi[5] << std::endl;
    }
}

void GraphcutSegmentationViewUi::RefreshGrapcutImage()
{
    Int3DImageType::Pointer itkImage = Int3DImageType::New();
    mitk::Image* mitkImage = dynamic_cast<mitk::Image *>(m_refImageNode->GetData());
    mitk::CastToItkImage<Int3DImageType>(mitkImage, itkImage);

    itk::Index<3> start;
    itk::Size<3>  size;

    start[0] = m_roi[0];
    start[1] = m_roi[2];
    start[2] = m_roi[4];
    size[0] = m_roi[1] - m_roi[0] + 1;
    size[1] = m_roi[3] - m_roi[2] + 1;
    size[2] = m_roi[5] - m_roi[4] + 1;
    itk::ImageRegion<3> region;
    region.SetIndex(start);
    region.SetSize(size);

    /*typedef itk::RegionOfInterestImageFilter<Int3DImageType, Int3DImageType> ROIFilterType;
    ROIFilterType::Pointer roiFilter = ROIFilterType::New();
    roiFilter->SetInput(itkImage);
    roiFilter->SetRegionOfInterest(region);
    roiFilter->Update();
    double min, max;
    ITKHelpers::GetImageScalarRange(roiFilter->GetOutput(), min, max);*/


    if (m_ui.DownSample->isChecked())
    {
        ITKHelpers::DownSampleImage(itkImage.GetPointer(), itkImage.GetPointer(), m_ui.ResampleLE->text().toDouble());
    }

    typedef itk::RescaleIntensityImageFilter<Int3DImageType, Float3DImageType> RescaleIntensityImageFilterType;
    RescaleIntensityImageFilterType::Pointer rescale = RescaleIntensityImageFilterType::New();
    rescale->SetInput(itkImage);
    rescale->SetOutputMinimum(0);
    rescale->SetOutputMaximum(255);
    rescale->Update();

    //m_graphcut->SetFirstTIme(true);
    m_graphcut->SetImage(rescale->GetOutput());
    //m_graphcut->SetRegion(region);
    //m_graphcut->SetScalarRange(mitkImage->GetScalarValueMin(), mitkImage->GetScalarValueMax());
    //m_graphcut->SetNumberOfHistogramBins(/*(range[1] - range[0])/2*/32);
}

void GraphcutSegmentationViewUi::SaveMask()
{
    UChar3DImageType* maskImage = m_graphcut->GetSegmentMask();
    Float3DImageType::Pointer itkImage = Float3DImageType::New();
    ITKHelpers::DeepCopy(maskImage, itkImage.GetPointer());

    Float3DImageType::Pointer image = Float3DImageType::New();
    mitk::CastToItkImage<Float3DImageType>(dynamic_cast<mitk::Image *>(m_refImageNode->GetData()), image);

    itkImage->SetOrigin(image->GetOrigin());
    itkImage->SetDirection(image->GetDirection());
    itkImage->SetSpacing(image->GetSpacing());

    //  ITKHelpers::SaveImage(itkImage.GetPointer(), "D:/temp/maskImge.mhd");

}

mitk::DataNode::Pointer GraphcutSegmentationViewUi::CreateSegmentationNode(mitk::Image* origin, const std::string& organName, const mitk::Color& color)
{
    if (!origin)
    {
        return nullptr;
    }

    mitk::PixelType pixelType(mitk::MakeScalarPixelType<mitk::Label::PixelType>());
    mitk::LabelSetImage::Pointer segmentation = mitk::LabelSetImage::New();
    if (origin->GetDimension() == 2)
    {
        const unsigned int dimensions[] = { origin->GetDimension(0), origin->GetDimension(1), 1 };
        segmentation->Initialize(pixelType, 3, dimensions);
        segmentation->AddLayer();
    }
    else
    {
        segmentation->Initialize(origin);
    }

    mitk::Label::Pointer label = mitk::Label::New();
    label->SetName(organName);
    label->SetColor(color);
    label->SetValue(1);
    segmentation->GetActiveLabelSet()->AddLabel(label);
    segmentation->GetActiveLabelSet()->SetActiveLabel(1);

    unsigned int byteSize = sizeof(mitk::Label::PixelType);

    if (segmentation->GetDimension() < 4)
    {
        for (unsigned int dim = 0; dim < segmentation->GetDimension(); ++dim)
        {
            byteSize *= segmentation->GetDimension(dim);
        }

        mitk::ImageWriteAccessor writeAccess(segmentation.GetPointer(), segmentation->GetVolumeData(0));

        memset(writeAccess.GetData(), 0, byteSize);
    }
    else
    {
        // if we have a time-resolved image we need to set memory to 0 for each time step
        for (unsigned int dim = 0; dim < 3; ++dim)
        {
            byteSize *= segmentation->GetDimension(dim);
        }

        for (unsigned int volumeNumber = 0; volumeNumber < segmentation->GetDimension(3); volumeNumber++)
        {
            mitk::ImageWriteAccessor writeAccess(segmentation.GetPointer(), segmentation->GetVolumeData(volumeNumber));

            memset(writeAccess.GetData(), 0, byteSize);
        }
    }

    if (origin->GetTimeGeometry())
    {
        mitk::TimeGeometry::Pointer originalGeometry = origin->GetTimeGeometry()->Clone();
        segmentation->SetTimeGeometry(originalGeometry);
    }
    else
    {
        //mitk::Tool::ErrorMessage("Original image does not have a 'Time sliced geometry'! Cannot create a segmentation.");
        return nullptr;
    }

    mitk::DataNode::Pointer segmentationNode = mitk::DataNode::New();
    segmentationNode->SetData(segmentation);

    // name
    segmentationNode->SetProperty("name", mitk::StringProperty::New(organName));

    // visualization properties
    segmentationNode->SetProperty("binary", mitk::BoolProperty::New(true));
    segmentationNode->SetProperty("color", mitk::ColorProperty::New(color));
    mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
    lut->SetType(mitk::LookupTable::MULTILABEL);
    mitk::LookupTableProperty::Pointer lutProp = mitk::LookupTableProperty::New();
    lutProp->SetLookupTable(lut);
    segmentationNode->SetProperty("LookupTable", lutProp);
    segmentationNode->SetProperty("texture interpolation", mitk::BoolProperty::New(false));
    segmentationNode->SetProperty("layer", mitk::IntProperty::New(10));
    segmentationNode->SetProperty("levelwindow", mitk::LevelWindowProperty::New(mitk::LevelWindow(0.5, 1)));
    segmentationNode->SetProperty("opacity", mitk::FloatProperty::New(0.3));
    segmentationNode->SetProperty("segmentation", mitk::BoolProperty::New(true));
    segmentationNode->SetProperty("reslice interpolation",
        mitk::VtkResliceInterpolationProperty::New());
    segmentationNode->SetProperty("showVolume", mitk::BoolProperty::New(true));

    return segmentationNode;
}

void GraphcutSegmentationViewUi::Segment()
{
    QTime time;

    time.start();
    //RefreshROI();
    EstimationSampleRate();
    RefreshGrapcutImage();
    RefreshSourceAndSink();

    if (m_ui.LambdaLE->text().toDouble()<0)
    {
        
        m_segOption.SetOrgan(m_ui.OrganType->currentText());
        m_graphcut->SetLambda(m_segOption.Lambda);
    }
    else
    {
        m_graphcut->SetLambda(m_ui.LambdaLE->text().toDouble());
    }

    m_graphcut->SetNumberOfHistogramBins(m_ui.HistogramBinsLE->text().toInt());
    m_graphcut->SetSources(m_sources);
    m_graphcut->SetSinks(m_sinks);
    m_graphcut->PerformSegmentation();

    UChar3DImageType::Pointer maskImage = UChar3DImageType::New();
    maskImage->Graft(m_graphcut->GetSegmentMask());


    if (m_ui.ConnectedDetect->isChecked())
    {
        //ITKHelpers::OpeningBinaryImage<UChar3DImageType, UChar3DImageType>(maskImage.GetPointer(), maskImage.GetPointer(), 1);
        ITKHelpers::ExtractConnectedContainsIndex(maskImage.GetPointer(), maskImage.GetPointer(), m_sources);
    }

    if (m_ui.SmoothResult->isChecked())
    {
        ITKHelpers::ResampleLabelImage<UChar3DImageType, UChar3DImageType, itk::GaussianInterpolateImageFunction>(
            maskImage.GetPointer(), maskImage.GetPointer(), 1.0 / m_ui.ResampleLE->text().toDouble());
    }
    else
    {
        ITKHelpers::ResampleLabelImage<UChar3DImageType, UChar3DImageType>(
            maskImage.GetPointer(), maskImage.GetPointer(), 1.0 / m_ui.ResampleLE->text().toDouble());
    }




    qDebug() << "Total time: " << time.elapsed() / 1000.0 << "s";

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitk::CastToMitkImage<UChar3DImageType>(maskImage, mitkImage);

    GetDataStorage()->Remove(
        GetDataStorage()->GetNamedNode(m_currentResultName.toStdString()));
    mitk::DataNode::Pointer resultNode = mitk::DataNode::New();
    resultNode->SetName(m_currentResultName.toStdString());
    resultNode->SetColor(1, 0, 0);
    resultNode->SetData(mitkImage);
    resultNode->SetBoolProperty("volumerendering", true);
    resultNode->Update();
    GetDataStorage()->Add(resultNode);
    resultNode->SetOpacity(0.8);
    m_currentResultNode = resultNode.GetPointer();

    RequestRenderWindowUpdate();
}


void GraphcutSegmentationViewUi::GenerateSurface()
{
    if (m_currentResultNode)
    {
        IQF_MitkSurfaceTool* surfaceTool = (IQF_MitkSurfaceTool*)m_pMain->GetInterfacePtr(QF_MitkSurface_Tool);
        vtkSmartPointer<vtkPolyData> surfaceData = vtkSmartPointer<vtkPolyData>::New();
        surfaceTool->ExtractSurface(m_currentResultNode, surfaceData, m_ui.SmoothTimesLE->text().toInt(),
            m_ui.ConnectedDetect->isChecked());

        mitk::DataNode::Pointer surfaceNode = mitk::DataNode::New();
        mitk::Surface::Pointer surface = mitk::Surface::New();
        surface->SetVtkPolyData(surfaceData);

        std::string name = m_currentResultNode->GetName();
        name.append("_surface");
        surfaceNode->SetData(surface);
        surfaceNode->SetName(name);
        surfaceNode->SetColor(m_segOption.Color);
        GetDataStorage()->Add(surfaceNode, m_currentResultNode);

    }
}

void GraphcutSegmentationViewUi::CreateView()
{
    m_pMain->Attach(this);

    m_ui.setupUi(this);

    m_ui.ImageSelector->SetDataStorage(GetDataStorage());
    m_ui.ImageSelector->SetPredicate(mitk::TNodePredicateDataType<mitk::Image>::New());
    connect(m_ui.ImageSelector, SIGNAL(OnSelectionChanged(const mitk::DataNode *)), this, SLOT(OnImageSelectionChanged(const mitk::DataNode *)));

    connect(m_ui.ResampleBtn, SIGNAL(clicked()), this, SLOT(Resmaple()));

    connect(m_ui.InitBtn, SIGNAL(clicked()), this, SLOT(Init()));
    connect(m_ui.RemoveAllBtn, SIGNAL(clicked()), this, SLOT(Reset()));
    connect(m_ui.SegmentBtn, SIGNAL(clicked()), this, SLOT(Segment()));
    connect(m_ui.SaveMaskBtn, SIGNAL(clicked()), this, SLOT(SaveMask()));
    connect(m_ui.GenerateSurfaceBtn, SIGNAL(clicked()), this, SLOT(GenerateSurface()));

    connect(m_ui.PaintBtn, SIGNAL(clicked()), this, SLOT(BeginPaint()));
    connect(m_ui.EraseBtn, SIGNAL(clicked()), this, SLOT(BeginWipe()));
    connect(m_ui.EndBtn, SIGNAL(clicked()), this, SLOT(EndTool()));

    connect(m_ui.Foreground, SIGNAL(clicked(bool)), this, SLOT(ForegroundChanged(bool)));
    connect(m_ui.Background, SIGNAL(clicked(bool)), this, SLOT(BackgroundChanged(bool)));

    connect(m_ui.PenSizeSlider, SIGNAL(valueChanged(int)), this, SLOT(PenSizeChanged(int)));

    connect(m_ui.LambdaLE, SIGNAL(textChanged(const QString &)), this, SLOT(LambdaChanged(const QString &)));
    connect(m_ui.HistogramBinsLE, SIGNAL(textChanged(const QString &)), this, SLOT(HistogramBinsChanged(const QString &)));


    m_pSegTool = (IQF_MitkSegmentationTool*)m_pMain->GetInterfacePtr(QF_MitkSegmentation_Tool);

    IQF_SegmentationMethodFactory* pFactory = (IQF_SegmentationMethodFactory*)m_pMain->GetInterfacePtr(QF_Segmentation_Factory);
    m_graphcut = pFactory->CreateGraphcutSegmentationMethod();

}

void GraphcutSegmentationViewUi::LambdaChanged(const QString &text)
{
    m_graphcut->SetLambda(text.toDouble());
    m_graphcut->Init();
}

void GraphcutSegmentationViewUi::HistogramBinsChanged(const QString &text)
{
    m_graphcut->SetNumberOfHistogramBins(text.toInt());
    m_graphcut->Init();
}

void GraphcutSegmentationViewUi::PenSizeChanged(int size)
{
    if (m_tool)
    {
        m_tool->SetSize(size);
    }
}

void GraphcutSegmentationViewUi::ForegroundChanged(bool checked)
{
     if (checked)
     {
         SwitchToForeground();
     }
}

void  GraphcutSegmentationViewUi::BackgroundChanged(bool checked)
{
    if (checked)
    {
        SwitchToBackground();
    }
}

void GraphcutSegmentationViewUi::BeginPaint()
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

void GraphcutSegmentationViewUi::BeginWipe()
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

void GraphcutSegmentationViewUi::EndTool()
{
    ChangeTool("");
}

void GraphcutSegmentationViewUi::OnContourValueChanged(int value)
{
    if (!m_resultSurfaceImageNode)
    {
        return;
    }
    mitk::Image* image = dynamic_cast<mitk::Image*>(m_resultSurfaceImageNode->GetData());
    if (!image)
    {
        return;
    }
    mitk::DataNode::Pointer surfaceNode = GetDataStorage()->GetNamedNode("Surface");
    if (!surfaceNode)
    {
        return;
    }

    vtkMarchingCubes *surfaceCreator = vtkMarchingCubes::New();
    surfaceCreator->SetInputData(image->GetVtkImageData());
    surfaceCreator->SetValue(value, value);
    surfaceCreator->Update();


    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(surfaceCreator->GetOutput());
    surface->SetOrigin(image->GetGeometry()->GetOrigin());
    surfaceNode->SetData(surface);

    RequestRenderWindowUpdate();
}

void GraphcutSegmentationViewUi::OnImageSelectionChanged(const mitk::DataNode* node)
{
    if (!node)
    {
        return;
    }
    m_refImageNode = (mitk::DataNode*)node;
    m_originMitkImage = dynamic_cast<mitk::Image*>(m_refImageNode->GetData());
    m_refImageNode->Modified();
    m_bInited = false;

}
