#ifndef IQF_GraphcutSegmentation_h__
#define IQF_GraphcutSegmentation_h__

#include "ITKImageTypeDef.h"

/********Demo**********/
/*
    //Create the method
    IQF_SegmentationMethodFactory* pFactory = (IQF_SegmentationMethodFactory*)pMain->GetInterfacePtr(QF_Segmentation_Factory);
    pGraphcut = pFactory->CreateGraphcutSegmentationMethod();

    //Init method
    pGraphcut->Init();
    pGraphcut->SetImage(inputImage);
    pGraphcut->SetSources(m_sources);
    pGraphcut->SetSinks(m_sinks);

    //Perform segmentation
    pGraphcut->PerformSegmentation();

    //Get the result
    UChar3DImageType::Pointer maskImage = UChar3DImageType::New();
    maskImage->Graft(pGraphcut->GetSegmentMask());

    pGraphcut->Release();
*/

class IQF_GraphcutSegmentation
{
public:
    virtual void Init()=0;
    virtual void PerformSegmentation() = 0;
    virtual void SetImage(const Float3DImageType* pF3DImage) = 0;
    virtual void SetSources(const IndexContainer& icSources) = 0;
    virtual void SetSinks(const IndexContainer& icSinks) = 0;    
    virtual UChar3DImageType* GetSegmentMask() = 0;

    virtual void SetLambda(float fLambda) = 0;
    virtual void SetNumberOfHistogramBins(int iNum) = 0;
    virtual void SetFirstTIme(bool bFirstTime) = 0;
    virtual void SetScalarRange(double dMin, double dMax) = 0;
    virtual void SetRegion(itk::ImageRegion<3> irRegion) = 0;
    virtual void Release() = 0;
};

#endif // IQF_GraphcutSegmentation_h__
