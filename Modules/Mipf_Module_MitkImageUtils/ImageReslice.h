#ifndef ImageReslice_h__
#define ImageReslice_h__

#pragma once
#include "MitkImageUtils/IQF_MitkImageReslice.h"

class ImageReslice:public IQF_MitkImageReslice
{
public:
    ImageReslice();
    ~ImageReslice();
    virtual bool GetReslicePlaneImageWithLevelWindow(mitk::DataNode* imageNode, const mitk::PlaneGeometry* worldGeometry, mitk::BaseRenderer* renderer, vtkImageData* output);
    virtual bool GetReslicePlaneImage(mitk::DataNode* imageNode, const mitk::PlaneGeometry* worldGeometry, vtkImageData* output);
    virtual bool GetReslicePlaneImage(vtkImageData* image, const mitk::PlaneGeometry* worldGeometry, vtkImageData* output);

};

#endif // ImageReslice_h__
