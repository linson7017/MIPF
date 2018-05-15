#include "DicomTagsView.h" 
#include "iqf_main.h"  
#include "qf_log.h"


#include <QFileDialog>
#include "QStringUtils.h"
#include "itkGDCMImageIO.h"
#include "itkMetaDataObject.h"
#include "ITKImageTypeDef.h"

#include "mitkImageCast.h"

#include "MapProperty.h"
  
DicomTagsView::DicomTagsView() :MitkPluginView() 
{
}
 
DicomTagsView::~DicomTagsView() 
{
}
 
void DicomTagsView::CreateView()
{
    m_ui.setupUi(this);
    m_ui.TagsTable->setHeaderLabels(QStringList() << "(Group,Element)" << "Description"<<"Value");

    connect(m_ui.LoadBtn, &QPushButton::clicked, this, &DicomTagsView::Load);
} 
 
WndHandle DicomTagsView::GetPluginHandle() 
{
    return this; 
}

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

mitk::DataNode::Pointer DicomTagsView::ReadDicom(const char* filename,const char* nodename)
{
    typedef itk::GDCMImageIO ImageIOType;
    ImageIOType::Pointer io = ImageIOType::New();
    if (io->CanReadFile(filename))
    {
        io->SetFileName(filename);
        io->ReadImageInformation();
        //read data and  create image
        mitk::DataNode::Pointer node = NULL;
        if (io->GetPixelType() == itk::ImageIOBase::SCALAR)
        {
            switch (io->GetComponentType())
            {
            case itk::ImageIOBase::INT:
            {
                //QF_INFO << "The scalar type of loading image is INT";
                node =  ReadDicomToITKImage<Int3DImageType>(io, nodename);
            }
            case itk::ImageIOBase::UCHAR:
            {
                //QF_INFO << "The scalar type of loading image is UCHAR";
                node = ReadDicomToITKImage<UChar3DImageType>(io, nodename);
            }
            case itk::ImageIOBase::FLOAT:
            {
                //QF_INFO << "The scalar type of loading image is FLOAT";
                node = ReadDicomToITKImage<Float3DImageType>(io, nodename);
            }
            case itk::ImageIOBase::USHORT:
            {
                //QF_INFO << "The scalar type of loading image is USHORT";
                node = ReadDicomToITKImage<UShort3DImageType>(io, nodename);
            }
            case itk::ImageIOBase::SHORT:
            {
                //QF_INFO << "The scalar type of loading image is SHORT";
                node = ReadDicomToITKImage<Short3DImageType>(io, nodename);
            }
            case itk::ImageIOBase::CHAR:
            {
                //QF_INFO << "The scalar type of loading image is CHAR";
                node = ReadDicomToITKImage<Char3DImageType>(io, nodename);
            }
            default:
                break;
            }
        }

        //parse tags
        if (node.IsNotNull())
        {
            const itk::MetaDataDictionary &dict = io->GetMetaDataDictionary();
            const TagMapType &propertyLookup = GetDICOMTagsToTagMap();
            mitk::MapProperty<std::string,std::string>::Pointer tagsMap = mitk::MapProperty<std::string, std::string>::New();
            auto dictIter = dict.Begin();
            while (dictIter != dict.End())
            {
                std::string value;
                if (itk::ExposeMetaData<std::string>(dict, dictIter->first, value))
                {
                    auto valuePosition = propertyLookup.find(dictIter->first);
                    if (valuePosition != propertyLookup.end())
                    {
                        std::string propertyKey = valuePosition->second;
                        node->SetProperty(propertyKey.c_str(), mitk::StringProperty::New(value));
                    }
                    else
                    {
                    }
                }
                else
                {
                    MITK_WARN << "Tag " << dictIter->first << " not read as string as expected. Ignoring...";
                }
                ++dictIter;
            }
        }
    }
    else
    {
        QF_WARN << "File " << filename << " can not be read!";
        return NULL;
    }
}

void DicomTagsView::Load()
{
    QString defaultOpenFilePath = "";
    defaultOpenFilePath = GetMitkReferenceInterface()->GetString("LastOpenDirectory");
    QString fileName = QFileDialog::getOpenFileName(NULL, "Open Dicom",
        defaultOpenFilePath,
        "DICOM (*.*)");
    if (fileName.isEmpty())
        return ;
    try
    {
        QFileInfo fi(fileName);
        if (fi.exists())
        {
            std::string localStr;
            mitk::DataNode::Pointer node = ReadDicom(QStringUtils::GetLocalString(fi.absoluteFilePath(), localStr),
                QStringUtils::GetLocalString(fi.fileName(), localStr));
            if (node.IsNotNull())
            {
                node->SetStringProperty("filename", fi.absoluteFilePath().toLocal8Bit().constData());
                GetDataStorage()->Add(node);
                IQF_MitkRenderWindow* pRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
                if (pRenderWindow)
                {
                    pRenderWindow->Reinit(node);
                }
                return ;
            }
            else
            {
                return ;
            }
        }
    }
    catch (const mitk::Exception& e)
    {
        QF_ERROR << e;
        return ;
    }
   GetMitkReferenceInterface()->SetString("LastOpenDirectory", QFileInfo(fileName).absolutePath().toStdString().c_str()); 
}

void DicomTagsView::Update(const char* szMessage, int iValue /* = 0 */, void* pValue /* = 0 */)
{
    if (strcmp(szMessage, MITK_MESSAGE_NODE_SELECTION_CHANGED) == 0)
    {
        std::vector<mitk::DataNode::Pointer> nodes = GetMitkDataManagerInterface()->GetSelectedNodes();
        if (nodes.size() == 0)
        {
            return;
        }
        mitk::DataNode* node = nodes.front().GetPointer();
        node->UpdateOutputInformation();

        if (!dynamic_cast<mitk::Image*>(node->GetData()))
        {
            return;
        }
    }
}

const DicomTagsView::TagMapType &DicomTagsView::GetDICOMTagsToTagMap()
{
    static bool initialized = false;
    static TagMapType dictionary;
    if (!initialized)
    {
        dictionary["0010|0010"] = "dicom.patient.PatientsName";
        dictionary["0010|0020"] = "dicom.patient.PatientID";
        dictionary["0010|0030"] = "dicom.patient.PatientsBirthDate";
        dictionary["0010|0040"] = "dicom.patient.PatientsSex";
        dictionary["0010|0032"] = "dicom.patient.PatientsBirthTime";
        dictionary["0010|1000"] = "dicom.patient.OtherPatientIDs";
        dictionary["0010|1001"] = "dicom.patient.OtherPatientNames";
        dictionary["0010|2160"] = "dicom.patient.EthnicGroup";
        dictionary["0010|4000"] = "dicom.patient.PatientComments";
        dictionary["0012|0062"] = "dicom.patient.PatientIdentityRemoved";
        dictionary["0012|0063"] = "dicom.patient.DeIdentificationMethod";

        // General Study module
        dictionary["0020|000d"] = "dicom.study.StudyInstanceUID";
        dictionary["0008|0020"] = "dicom.study.StudyDate";
        dictionary["0008|0030"] = "dicom.study.StudyTime";
        dictionary["0008|0090"] = "dicom.study.ReferringPhysiciansName";
        dictionary["0020|0010"] = "dicom.study.StudyID";
        dictionary["0008|0050"] = "dicom.study.AccessionNumber";
        dictionary["0008|1030"] = "dicom.study.StudyDescription";
        dictionary["0008|1048"] = "dicom.study.PhysiciansOfRecord";
        dictionary["0008|1060"] = "dicom.study.NameOfPhysicianReadingStudy";

        // General Series module
        dictionary["0008|0060"] = "dicom.series.Modality";
        dictionary["0020|000e"] = "dicom.series.SeriesInstanceUID";
        dictionary["0020|0011"] = "dicom.series.SeriesNumber";
        dictionary["0020|0060"] = "dicom.series.Laterality";
        dictionary["0008|0021"] = "dicom.series.SeriesDate";
        dictionary["0008|0031"] = "dicom.series.SeriesTime";
        dictionary["0008|1050"] = "dicom.series.PerformingPhysiciansName";
        dictionary["0018|1030"] = "dicom.series.ProtocolName";
        dictionary["0008|103e"] = "dicom.series.SeriesDescription";
        dictionary["0008|1070"] = "dicom.series.OperatorsName";
        dictionary["0018|0015"] = "dicom.series.BodyPartExamined";
        dictionary["0018|5100"] = "dicom.series.PatientPosition";
        dictionary["0028|0108"] = "dicom.series.SmallestPixelValueInSeries";
        dictionary["0028|0109"] = "dicom.series.LargestPixelValueInSeries";

        // VOI LUT module
        dictionary["0028|1050"] = "dicom.voilut.WindowCenter";
        dictionary["0028|1051"] = "dicom.voilut.WindowWidth";
        dictionary["0028|1055"] = "dicom.voilut.WindowCenterAndWidthExplanation";

        // Image Pixel module
        dictionary["0028|0004"] = "dicom.pixel.PhotometricInterpretation";
        dictionary["0028|0010"] = "dicom.pixel.Rows";
        dictionary["0028|0011"] = "dicom.pixel.Columns";

        // Image Plane module
        dictionary["0028|0030"] = "dicom.PixelSpacing";
        dictionary["0018|1164"] = "dicom.ImagerPixelSpacing";

        initialized = true;
    }

    return dictionary;
}