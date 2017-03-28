#ifndef UTIL_H
#define UTIL_H
//#include <Vector3>
#include <vtkImageData.h>
#include <vtkPointData.h>
#include <valarray>
#include "VTKImageProperties.h"

typedef vtkImageData InternalImageType;
typedef vtkImageData BinaryImageType;

//typedef Vector3 Vector3;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

typedef VTKImageProperties MedicalImageProperties;
////////////////gaus//////////////////////
const unsigned int hessian2DOrder[3][2]   = {{2,0},{1,1},{0,2}};          // Hxx, Hxy, Hyy
const unsigned int hessian3DOrder[6][3]   = {{2,0,0},{1,1,0},{1,0,1},     // Hxx, Hxy, Hxz
                                                     {0,2,0},{0,1,1},     //      Hyy, Hyz
                                                             {0,0,2}};    //           Hzz


unsigned int getKernelSize(double sigma, unsigned int order);

void get1DGaussKernel(double sigma, std::valarray<double>& kernel, unsigned int order, double voxelSize = 1.0, bool multiScaleNormalize = false);


#endif // UTIL_H
