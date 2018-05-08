#include "CQF_DSATool.h"

#include "itkGDCMImageIO.h"
#include "ITKImageTypeDef.h"


//mitk
#include "mitkImageCast.h"

template <class ImageType>
mitk::DataNode::Pointer ReadDicomToITKImage(itk::GDCMImageIO* io, const char* name = "")
{
    std::string frameNum;
    bool success = io->GetValueFromTag("0028|0008", frameNum);
    double spacing[3] = { 0,0,0 };
    int dimens[3] = { 1,1,1 };
    double origin[3] = { 0,0,0 };
    spacing[0] = io->GetSpacing(0);
    spacing[1] = io->GetSpacing(1);
    spacing[2] = io->GetSpacing(2);
    dimens[0] = io->GetDimensions(0);
    dimens[1] = io->GetDimensions(1);
    if (frameNum.empty())
    {
        dimens[2] = io->GetDimensions(2);
    }
    else
    {
        dimens[2] = atoi(frameNum.c_str());
    }
    origin[0] = io->GetOrigin(0);
    origin[1] = io->GetOrigin(1);
    origin[2] = io->GetOrigin(2);

    ImageType::Pointer image = ImageType::New();
    ImageType::IndexType corner;
    corner[0] = origin[0];
    corner[1] = origin[1];
    corner[2] = origin[2];
    ImageType::SizeType size;
    size[0] = dimens[0];
    size[1] = dimens[1];
    size[2] = dimens[2];
    ImageType::RegionType region(corner, size);
    image->SetRegions(region);
    image->SetSpacing(spacing);
    image->SetOrigin(origin);
    image->Allocate();
    io->Read(image->GetBufferPointer());

    mitk::Image::Pointer mitkImage = mitk::Image::New();
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    mitk::CastToMitkImage(image, mitkImage);
    node->SetData(mitkImage);
    node->SetName(name);
    return node;
}


CQF_DSATool::CQF_DSATool()
{
}


CQF_DSATool::~CQF_DSATool()
{
}

mitk::DataNode::Pointer CQF_DSATool::LoadDSADicomFile(const char* szFileName,const char* szNodeName)
{
    typedef itk::GDCMImageIO ImageIOType;
    ImageIOType::Pointer io = ImageIOType::New();
    if (io->CanReadFile(szFileName))
    {
        io->SetFileName(szFileName);
        io->ReadImageInformation();
        std::string value;
        if (io->GetPixelType() == itk::ImageIOBase::SCALAR)
        {
            switch (io->GetComponentType())
            {
            case itk::ImageIOBase::INT:
            {
                MITK_INFO << "The scalar type of loading image is INT";
                return ReadDicomToITKImage<Int3DImageType>(io, szNodeName);
            }
            case itk::ImageIOBase::UCHAR:
            {
                MITK_INFO << "The scalar type of loading image is UCHAR";
                return ReadDicomToITKImage<UChar3DImageType>(io, szNodeName);
            }
            case itk::ImageIOBase::FLOAT:
            {
                MITK_INFO << "The scalar type of loading image is FLOAT";
                return ReadDicomToITKImage<Float3DImageType>(io, szNodeName);
            }
            case itk::ImageIOBase::USHORT:
            {
                MITK_INFO << "The scalar type of loading image is USHORT";
                return ReadDicomToITKImage<UShort3DImageType>(io, szNodeName);
            }
            case itk::ImageIOBase::SHORT:
            {
                MITK_INFO << "The scalar type of loading image is SHORT";
                return ReadDicomToITKImage<Short3DImageType>(io, szNodeName);
            }
            case itk::ImageIOBase::CHAR:
            {
                MITK_INFO << "The scalar type of loading image is CHAR";
                return ReadDicomToITKImage<Char3DImageType>(io, szNodeName);
            }
            default:
                return NULL;
                break;
            }
        }
    }
    else
    {    
        MITK_WARN << "File " << szFileName << " can not be read!";
        return NULL;
    }
}

bool CQF_DSATool::SaveDSADicomFile(mitk::Image* pImage, const char* szFileName)
{
    if (strcmp(szFileName,"")==0)
    {
        return false;
    }
    typedef itk::GDCMImageIO ImageIOType;
    ImageIOType::Pointer io = ImageIOType::New();
    if (io->CanReadFile(szFileName))
    {
        io->SetFileName(szFileName);
        io->ReadImageInformation();
        std::string value;
        io->Write(pImage->GetVtkImageData()->GetScalarPointer());
        return true;
    }
    else
    {
        MITK_WARN << "File " << szFileName << " can not be read!";
        return false;
    }
}
