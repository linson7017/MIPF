/********************************************************************
	FileName:    IQF_RSSSegmentation.h
	Author:        Ling Song
	Date:           Month 3 ; Year 2018
	Purpose:	     
*********************************************************************/
#ifndef IQF_RSSSegmentation_h__
#define IQF_RSSSegmentation_h__

#include "ITKImageTypeDef.h"
#include "IQF_Object.h"

class IQF_RSSSegmentation :public IQF_Object
{
public:
    virtual void SetImage(Float3DImageType* pInputImage) = 0;
    virtual void SetNumIter(int iIteratorNumber) = 0;
    virtual void SetMaxVolume(int iMaxVolume) = 0;
    virtual void SetInputLabeledImage(UShort3DImageType* pLabelImage) = 0;
    virtual void SetMaxRunningTime(int iMaxTime) = 0;
    virtual void SetIntensityHomogeneity(double dHomogeneity) = 0;
    virtual void SetCurvatureWeight(double dCurvatureWeight) = 0;
    virtual void PerformSegmentation() = 0;
    virtual Float3DImageType* GetResult() = 0;
    virtual void GetFinalMask(Float3DImageType* pInput, UShort3DImageType* pOutput, int iForegroundValue, float fUpperThreshold) = 0;
};
#endif // IQF_RSSSegmentation_h__