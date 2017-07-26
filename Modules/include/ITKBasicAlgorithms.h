#ifndef ITKBasicAlgorithms_h__
#define ITKBasicAlgorithms_h__

#include "itkMinimumMaximumImageCalculator.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkLabelShapeKeepNObjectsImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkLabelShapeOpeningImageFilter.h"
#include "itkBinaryShapeOpeningImageFilter.h"
#include "itkBinaryImageToShapeLabelMapFilter.h"

#include "ITKImageTypeDef.h"


namespace ITKBasicAlgorithms 
{


template<typename TInputImage, typename TPixelType>
void GetImageScalarRange(const TInputImage* image, TPixelType& rangeMin, TPixelType& rangeMax)
{
    typedef itk::MinimumMaximumImageCalculator <TInputImage>
        ImageCalculatorFilterType;
    ImageCalculatorFilterType::Pointer imageCalculatorFilter
        = ImageCalculatorFilterType::New();
    imageCalculatorFilter->SetImage(image);
    imageCalculatorFilter->Compute();

    rangeMin = (TPixelType)imageCalculatorFilter->GetMinimum();
    rangeMax = (TPixelType)imageCalculatorFilter->GetMaximum();
}


template <class TInput, class TOutput>
void ExtractLargestConnected(TInput* input,TOutput* output)
{
    TInput::PixelType min, max;
    GetImageScalarRange<TInput>(input, min, max);

    typedef itk::ConnectedComponentImageFilter <TInput, UInt3DImageType >
        ConnectedComponentImageFilterType;
    ConnectedComponentImageFilterType::Pointer connected = ConnectedComponentImageFilterType::New();
    connected->SetInput(input);
    connected->Update();

    typedef itk::LabelShapeKeepNObjectsImageFilter< UInt3DImageType > LabelShapeKeepNObjectsImageFilterType;
    LabelShapeKeepNObjectsImageFilterType::Pointer labelShapeKeepNObjectsImageFilter = LabelShapeKeepNObjectsImageFilterType::New();
    labelShapeKeepNObjectsImageFilter->SetInput(connected->GetOutput());
    labelShapeKeepNObjectsImageFilter->SetBackgroundValue(0);
    labelShapeKeepNObjectsImageFilter->SetNumberOfObjects(1);
    labelShapeKeepNObjectsImageFilter->SetAttribute(LabelShapeKeepNObjectsImageFilterType::LabelObjectType::NUMBER_OF_PIXELS);

    typedef itk::RescaleIntensityImageFilter< UInt3DImageType, TOutput > RescaleFilterType;
    RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetOutputMinimum(min);
    rescaleFilter->SetOutputMaximum(max);
    rescaleFilter->SetInput(labelShapeKeepNObjectsImageFilter->GetOutput());
    rescaleFilter->Update();

    output->Graft(rescaleFilter->GetOutput());
}

template <class TInput, class TOutput, class TAttributeType = itk::ShapeLabelObject<int, 3>::AttributeType>
void ExtractConnectedLargerThan(TInput* input, TOutput* output, double size=1000.0, 
    TAttributeType attributeType = itk::ShapeLabelObject<int, 3>::NUMBER_OF_PIXELS, bool reverse = false)
{
    // Create a ShapeLabelMap from the image
    typedef itk::BinaryImageToShapeLabelMapFilter<TInput> BinaryImageToShapeLabelMapFilterType;
    BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
    binaryImageToShapeLabelMapFilter->SetFullyConnected(true);
    binaryImageToShapeLabelMapFilter->SetInputForegroundValue(1);
    binaryImageToShapeLabelMapFilter->SetInput(input);
    binaryImageToShapeLabelMapFilter->Update();

    // Remove label objects that have NUMBER_OF_PIXELS smaller than size
    typedef itk::ShapeOpeningLabelMapFilter< BinaryImageToShapeLabelMapFilterType::OutputImageType > ShapeOpeningLabelMapFilterType;
    ShapeOpeningLabelMapFilterType::Pointer shapeOpeningLabelMapFilter = ShapeOpeningLabelMapFilterType::New();
    shapeOpeningLabelMapFilter->SetInput(binaryImageToShapeLabelMapFilter->GetOutput());
    shapeOpeningLabelMapFilter->SetLambda(size);
    shapeOpeningLabelMapFilter->SetReverseOrdering(reverse);
    shapeOpeningLabelMapFilter->SetAttribute(attributeType);
    shapeOpeningLabelMapFilter->Update();

    // Create a label image
    typedef itk::LabelMapToLabelImageFilter<BinaryImageToShapeLabelMapFilterType::OutputImageType, TOutput> LabelMapToLabelImageFilterType;
    LabelMapToLabelImageFilterType::Pointer labelMapToLabelImageFilter = LabelMapToLabelImageFilterType::New();
    labelMapToLabelImageFilter->SetInput(shapeOpeningLabelMapFilter->GetOutput());
    labelMapToLabelImageFilter->Update();

    typedef itk::BinaryThresholdImageFilter<TOutput, TOutput> BinaryThresholdImageFilterType;
    BinaryThresholdImageFilterType::Pointer binaryThresholdFilter = BinaryThresholdImageFilterType::New();
    binaryThresholdFilter->SetInput(labelMapToLabelImageFilter->GetOutput());
    binaryThresholdFilter->SetLowerThreshold(1);
    binaryThresholdFilter->SetInsideValue(1);
    binaryThresholdFilter->SetOutsideValue(0);
    binaryThresholdFilter->Update();

    output->Graft(binaryThresholdFilter->GetOutput());
}

template <class TInput, class TOutput>
void ExtractConnectedContainsIndex(TInput* input, TOutput* output, IndexContainer indexes)
{
    // Create a ShapeLabelMap from the image
    typedef itk::BinaryImageToShapeLabelMapFilter<TInput> BinaryImageToShapeLabelMapFilterType;
    BinaryImageToShapeLabelMapFilterType::Pointer binaryImageToShapeLabelMapFilter = BinaryImageToShapeLabelMapFilterType::New();
    binaryImageToShapeLabelMapFilter->SetFullyConnected(true);
    binaryImageToShapeLabelMapFilter->SetInputForegroundValue(1);
    binaryImageToShapeLabelMapFilter->SetInput(input);
    binaryImageToShapeLabelMapFilter->Update();

    double label = -1.0;
    for (IndexContainer::iterator it = indexes.begin();it!=indexes.end();it++)
    {
        for (unsigned int i = 0; i < binaryImageToShapeLabelMapFilter->GetOutput()->GetNumberOfLabelObjects(); i++)
        {
            BinaryImageToShapeLabelMapFilterType::OutputImageType::LabelObjectType* labelObject =
                binaryImageToShapeLabelMapFilter->GetOutput()->GetNthLabelObject(i);
            if (labelObject->HasIndex(*it) )
            {
                label = labelObject->GetNumberOfPixels();
                break;
            }
        }
        if (label>0)
        {
            break;
        }
    }
    ExtractConnectedLargerThan(input,output,label, BinaryImageToShapeLabelMapFilterType::LabelObjectType::NUMBER_OF_PIXELS);
    ExtractConnectedLargerThan(output, output, label, BinaryImageToShapeLabelMapFilterType::LabelObjectType::NUMBER_OF_PIXELS,true);
}


}


#endif // ITKBasicAlgorithms_h__
