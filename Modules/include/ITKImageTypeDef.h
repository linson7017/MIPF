#ifndef ITKImageTypeDef_h__
#define ITKImageTypeDef_h__
#include <itkImage.h>

typedef float FloatPixelType;
typedef int IntPixelType;
typedef char CharPixelType;
typedef short ShortPixelType;

typedef unsigned char UCharPixelType;
typedef unsigned int UIntPixelType;
typedef unsigned short UShortPixelType;

typedef itk::FixedArray< int, 1 > IntVectorType1;


const unsigned int Dimens3D = 3;
const unsigned int Dimens2D = 2;
const unsigned int Dimens1D = 1;

const unsigned char Pixel1D = 1;
const unsigned char Pixel2D = 2;
const unsigned char Pixel3D = 3;
const unsigned char Pixel4D = 4;

//image type
typedef itk::Image< FloatPixelType, Dimens3D >      Float3DImageType;
typedef itk::Image< IntPixelType, Dimens3D >        Int3DImageType;
typedef itk::Image< CharPixelType, Dimens3D >      Char3DImageType;
typedef itk::Image< ShortPixelType, Dimens3D >        Short3DImageType;
typedef itk::Image< UCharPixelType, Dimens3D >      UChar3DImageType;
typedef itk::Image< UIntPixelType, Dimens3D >       UInt3DImageType;
typedef itk::Image< UShortPixelType, Dimens3D >       UShort3DImageType;

typedef itk::Image< IntVectorType1, 3 > IntArray3DImageType;

typedef itk::Image< FloatPixelType, Dimens2D >      Float2DImageType;
typedef itk::Image< IntPixelType, Dimens2D >        Int2DImageType;
typedef itk::Image< CharPixelType, Dimens2D >      Char2DImageType;
typedef itk::Image< ShortPixelType, Dimens2D >        Short2DImageType;
typedef itk::Image< UCharPixelType, Dimens2D >      UChar2DImageType;
typedef itk::Image< UIntPixelType, Dimens2D >       UInt2DImageType;
typedef itk::Image< UShortPixelType, Dimens2D >       UShort2DImageType;

//image pointer type
typedef Float3DImageType::Pointer                   Float3DImagePointerType;
typedef Int3DImageType::Pointer                     Int3DImagePointerType;
typedef Char3DImageType::Pointer                   Char3DImagePointerType;
typedef Short3DImageType::Pointer                     Short3DImagePointerType;
typedef UChar3DImageType::Pointer                   UChar3DImagePointerType;
typedef UInt3DImageType::Pointer                    UInt3DImagePointerType;
typedef UShort3DImageType::Pointer                    UShort3DImagePointerType;

typedef Float2DImageType::Pointer                   Float2DImagePointerType;
typedef Int2DImageType::Pointer                     Int2DImagePointerType;
typedef Char2DImageType::Pointer                   Char2DImagePointerType;
typedef Short2DImageType::Pointer                     Short2DImagePointerType;
typedef UChar2DImageType::Pointer                   UChar2DImagePointerType;
typedef UInt2DImageType::Pointer                    UInt2DImagePointerType;
typedef UShort2DImageType::Pointer                    UShort2DImagePointerType;

//image const pointer type
typedef Float3DImageType::ConstPointer                   Float3DImageConstPointerType;
typedef Int3DImageType::ConstPointer                     Int3DImageConstPointerType;
typedef Char3DImageType::ConstPointer                   Char3DImageConstPointerType;
typedef Short3DImageType::ConstPointer                     Short3DImageConstPointerType;
typedef UChar3DImageType::ConstPointer                   UChar3DImageConstPointerType;
typedef UInt3DImageType::ConstPointer                    UInt3DImageConstPointerType;
typedef UShort3DImageType::ConstPointer                    UShort3DImageConstPointerType;

typedef Float2DImageType::ConstPointer                   Float2DImageConstPointerType;
typedef UChar2DImageType::ConstPointer                   UChar2DImageConstPointerType;
typedef Char2DImageType::ConstPointer                   Char2DImageConstPointerType;
typedef Short2DImageType::ConstPointer                     Short2DImageConstPointerType;
typedef Int2DImageType::ConstPointer                     Int2DImageConstPointerType;
typedef UInt2DImageType::ConstPointer                    UInt2DImageConstPointerType;
typedef UShort2DImageType::ConstPointer                    UShort2DImageConstPointerType;


//image region type
typedef Float3DImageType::RegionType                   Float3DImageRegionType;
typedef UChar3DImageType::RegionType                   UChar3DImageRegionType;
typedef Int3DImageType::RegionType                     Int3DImageRegionType;
typedef UInt3DImageType::RegionType                    UInt3DImageRegionType;

typedef Float2DImageType::RegionType                   Float2DImageRegionType;
typedef UChar2DImageType::RegionType                   UChar2DImageRegionType;
typedef Int2DImageType::RegionType                     Int2DImageRegionType;
typedef UInt2DImageType::RegionType                    UInt2DImageRegionType;

//image index type
typedef Float3DImageType::IndexType                   Float3DImageIndexType;
typedef UChar3DImageType::IndexType                   UChar3DImageIndexType;
typedef Int3DImageType::IndexType                     Int3DImageIndexType;
typedef UInt3DImageType::IndexType                    UInt3DImageIndexType;
typedef UShort3DImageType::IndexType                    UShort3DImageIndexType;

typedef Float2DImageType::IndexType                   Float2DImageIndexType;
typedef UChar2DImageType::IndexType                   UChar2DImageIndexType;
typedef Int2DImageType::IndexType                     Int2DImageIndexType;
typedef UInt2DImageType::IndexType                    UInt2DImageIndexType;
typedef UShort2DImageType::IndexType                    UShort2DImageIndexType;


struct IndexSortCriterion {
public:
    bool operator() (const itk::Index<3> &a, const itk::Index<3> &b) const {
        if (a.GetElement(0) < b.GetElement(0))
            return true;
        else if (a.GetElement(0) == b.GetElement(0))
        {
            if (a.GetElement(1) < b.GetElement(1))
            {
                return true;
            }
            else if (a.GetElement(1) == b.GetElement(1))
            {
                if (a.GetElement(2) < b.GetElement(2))
                {
                    return true;
                }
                else
                    return false;
            }
            else
                return false;
        }
        else
            return false;
    }
};

typedef std::set<itk::Index<3>, IndexSortCriterion > IndexContainer;

#endif // ITKImageTypeDef_h__
