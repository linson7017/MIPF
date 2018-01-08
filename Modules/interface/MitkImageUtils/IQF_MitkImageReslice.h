#ifndef IQF_MitkImageReslice_h__
#define IQF_MitkImageReslice_h__

#include "iconfig.h"

const char QF_MitkImageUtils_ImageReslice[] = "QF_MitkImageUtils_ImageReslice";

namespace mitk
{
    class DataNode;
    class PlaneGeometry;
    class BaseRenderer;
    class Image;
}

class vtkImageData;

class IQF_MitkImageReslice
{
    BUILD_INTERFACE(IQF_MitkImageReslice)
public:
    virtual bool GetReslicePlaneImageWithLevelWindow(mitk::DataNode* imageNode, const mitk::PlaneGeometry* worldGeometry, mitk::BaseRenderer* renderer, vtkImageData* output) = 0;

};


#endif // IQF_MitkImageReslice_h__
