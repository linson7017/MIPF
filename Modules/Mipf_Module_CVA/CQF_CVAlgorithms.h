#ifndef CQF_CVAlgorithms_h__
#define CQF_CVAlgorithms_h__
/********************************************************************
	FileName:    CQF_CVAlgorithms
	Author:        Ling Song
	Date:           Month 3 ; Year 2018
	Purpose:	     
*********************************************************************/
#include "CVA/IQF_CVAlgorithms.h"

//#include "imageAlgorithm.h"

class CQF_CVAlgorithms  :public IQF_CVAlgorithms
{
public:
    CQF_CVAlgorithms();
    ~CQF_CVAlgorithms();
     //interfaces
    virtual void GenerateVesselSurface(vtkImageData* pInputImage, vtkPoints* pSeeds,double* dvThreshold,vtkPolyData* pOutput, int iImageType = 0, bool bNeedRecalculateSegmentation = true);
    virtual void Release() { delete this; }
protected:
    void SegmentVessels(vtkImageData* pInputImage, vtkPoints* pSeeds, double* dvThreshold, vtkImageData* pOutputImage, int iImageType = 0, bool bNeedRecalculateSegmentation=true);
    void CalculateVesselness(vtkImageData* pInputImage, vtkImageData* pOutputImage, double* dvThreshold);
    void VesselEnhance(vtkImageData* pInputImage, vtkImageData* pOutputImage, double* dvThreshold);
    void AautoSelectRange(vtkImageData* pInputImage,double* outputRange,int modality=0);
private:

};

#endif // CQF_CVAlgorithms_h__
