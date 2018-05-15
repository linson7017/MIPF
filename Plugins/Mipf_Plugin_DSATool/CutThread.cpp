#include "CutThread.h"
#include <QFileDialog>
#include <QTextCodec>

#include "mitkImageCast.h"


#include "ITKImageTypeDef.h"
#include "ITK_Helpers.h"  

#include <QDebug>

#include "CVA/IQF_DSATool.h"
#include "cva/cva_command_def.h"

#include "QStringUtils.h"


CutThread::CutThread(QF::IQF_Main* pMain):m_pMain(pMain)
{
    m_pDSATool = (IQF_DSATool*)m_pMain->GetInterfacePtr(QF_INTERFACE_DSA_TOOL);

    m_pSubject = QF::QF_CreateSubjectObject();
}


CutThread::~CutThread()
{
}


void CutThread::Start(const QFileInfoList& list, const QString& openDir, const QString& saveDir)
{
    QDir dir;
    foreach(QFileInfo info, list)
    {
        QString dcmDirPath = info.absoluteFilePath();
        dcmDirPath.replace(openDir, saveDir);
        dir.mkpath(dcmDirPath);
        ProcessAndSaveDSA(info.absoluteFilePath(), dcmDirPath);
    }
    emit SignalEnd();
}


template <class PixelType>
mitk::Image::Pointer CutThread::CutImage(mitk::Image* mitkImage)
{
    typedef  itk::Image<PixelType, 2> ImageType2D;
    typedef  itk::Image<PixelType, 3> ImageType3D;
    Int3DImageType::Pointer itkImage;
    mitk::CastToItkImage(mitkImage, itkImage);
    Int3DImageType::SizeType inSize = itkImage->GetLargestPossibleRegion().GetSize();
    Int2DImageType::Pointer originSlice = Int2DImageType::New();
    ITKHelpers::Extract2DSlice<Int3DImageType, Int2DImageType>(itkImage, originSlice, 0);
    Int3DImageType::Pointer resutlImage = Int3DImageType::New();
    resutlImage->Graft(itkImage);
    for (int i = 0; i < inSize[2]; i++)
    {
        Int2DImageType::Pointer targetSlice = Int2DImageType::New();
        ITKHelpers::Extract2DSlice<Int3DImageType, Int2DImageType>(itkImage, targetSlice, i);
        typedef itk::SubtractImageFilter <Int2DImageType, Int2DImageType >
            SubtractImageFilterType;
        SubtractImageFilterType::Pointer subtractFilter
            = SubtractImageFilterType::New();
        subtractFilter->SetInput1(targetSlice);
        subtractFilter->SetInput2(originSlice);
        subtractFilter->Update();
        ITKHelpers::Assign2DSlice<Int3DImageType, Int2DImageType>(resutlImage, subtractFilter->GetOutput(), i);
        emit SignalCurrentResult(subtractFilter->GetOutput());
    }
    typedef itk::RescaleIntensityImageFilter< Int3DImageType, ImageType3D > RescaleFilterType;
    RescaleFilterType::Pointer rescaleFilter = RescaleFilterType::New();
    rescaleFilter->SetInput(resutlImage);
    rescaleFilter->SetOutputMinimum(mitkImage->GetScalarValueMin());
    rescaleFilter->SetOutputMaximum(mitkImage->GetScalarValueMax());
    rescaleFilter->Update();

    mitk::Image::Pointer mitkResultImage = mitk::Image::New();
    mitk::CastToMitkImage(rescaleFilter->GetOutput(), mitkResultImage);
    return mitkResultImage;

}

void CutThread::ProcessAndSaveDSA(QString openDir, QString saveDir)
{
    QDir dir(openDir);
    QFileInfoList fileList = dir.entryInfoList(QDir::Files);
    std::string localStr;
    foreach(QFileInfo info, fileList)
    {
        QString logStr;
        QString dcmFilePath = info.absoluteFilePath();
        mitk::DataNode::Pointer node = m_pDSATool->LoadDSADicomFile(QStringUtils::GetLocalString(dcmFilePath, localStr));
        if (node.IsNull())
        {
            emit SignalLog("<font color=red weight=bold size=4>Load file " + dcmFilePath + " failed !</font>", dcmFilePath);
            continue;
        }
        emit SignalLog("<font color=green>Load file " + dcmFilePath + " successfully !</font>", dcmFilePath);
        mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(node->GetData());
        mitk::Image::Pointer mitkResultImage = nullptr;
        switch (mitkImage->GetVtkImageData()->GetScalarType())
        {
        case VTK_INT:
            mitkResultImage = CutImage<int>(mitkImage);
            break;
        case VTK_CHAR:
            mitkResultImage = CutImage<char>(mitkImage);
            break;
        case VTK_SHORT:
            mitkResultImage = CutImage<short>(mitkImage);
            break;
        case VTK_UNSIGNED_INT:
            mitkResultImage = CutImage<unsigned int>(mitkImage);
            break;
        case VTK_UNSIGNED_CHAR:
            mitkResultImage = CutImage<unsigned char>(mitkImage);
            break;
        case VTK_UNSIGNED_SHORT:
            mitkResultImage = CutImage<unsigned short>(mitkImage);
            break;
        case VTK_FLOAT:
            mitkResultImage = CutImage<float>(mitkImage);
            break;
        default:
            break;
        }
        if (mitkResultImage.IsNull())
        {
            emit SignalLog("<font color=red weight=bold size=4>Cut image weight=bold size=4" + dcmFilePath + " Failed !</font>", dcmFilePath);
            continue;
        }
        emit SignalLog("<font color=green>Cut image " + dcmFilePath + " successfully !</font>", dcmFilePath);
        QString dcmSaveFilePath = dcmFilePath;
        dcmSaveFilePath.replace(openDir, saveDir);
        if (dir.exists(dcmSaveFilePath))
        {
            dir.remove(dcmSaveFilePath);
        }
        QFile::copy(dcmFilePath, dcmSaveFilePath);
        std::map<std::string, std::string> dict;
        dict["0028|1050"] = "1600";
        dict["0028|1051"] = "2100";
        if (m_pDSATool->SaveDSADicomFile(mitkResultImage, QStringUtils::GetLocalString(dcmSaveFilePath, localStr), dict))
        {
            emit SignalLog("<font color=green>Save result to " + dcmSaveFilePath + " !</font><br>", dcmFilePath);
        }
        else
        {
            emit SignalLog("<font color=red weight=bold size=4>Save result to " + dcmSaveFilePath + "Failed !</font><br>", dcmFilePath);
        }
    }
}