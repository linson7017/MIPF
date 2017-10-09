#include "PCAShapeModelEstimatorView.h"

//qt
#include <QFileDialog>

//itk
#include "itkImage.h"
#include "itkImagePCAShapeModelEstimator.h"
#include "itkBinaryThresholdImageFilter.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkSignedDanielssonDistanceMapImageFilter.h"

#include "itkShiftScaleImageFilter.h"
#include "itkNumericSeriesFileNames.h"

//mitk
#include "mitkImage.h"
#include "mitkImageCast.h"
#include "mitkDataNode.h"
#include "mitkRenderingManager.h"

//mipf
#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkRenderWindow.h"

//qf
#include "iqf_main.h"


#include "dcmtk/dcmdata/dctk.h"


PCAShapeModelEstimatorView::PCAShapeModelEstimatorView(QF::IQF_Main* pMain, QWidget* parent):QWidget(parent),m_pMain(pMain)
{
    m_ui.setupUi(this);
    m_ui.ImageList->setHeaderLabels(QStringList() << "Name" << "Path");
    connect(m_ui.BrowseBtn, &QPushButton::clicked, this, &PCAShapeModelEstimatorView::BrowseFile);
    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &PCAShapeModelEstimatorView::Apply);

}


PCAShapeModelEstimatorView::~PCAShapeModelEstimatorView()
{
}

void PCAShapeModelEstimatorView::Apply()
{
    //parameters
    int NUMPC = 3;

    //defining internal pixel type
    typedef   float           InternalPixelType;
    const     unsigned int    Dimension = 3;
    typedef itk::Image< InternalPixelType, Dimension >  InternalImageType;

    typedef  itk::ImageFileReader< InternalImageType > ReaderType;

    //defining image type for threshold filter and image
    typedef float BinaryOutputPixelType;
    typedef itk::Image< BinaryOutputPixelType, Dimension > OutputImageType;
    typedef itk::BinaryThresholdImageFilter< InternalImageType, OutputImageType>  ThresholdingFilterType;


    //defining types for danielsson distance map image filter
    typedef float MapOutputPixelType;
    typedef itk::Image< BinaryOutputPixelType, Dimension> MapInputImageType;
    typedef itk::Image< MapOutputPixelType, Dimension> MapOutputImageType;
    typedef itk::SignedDanielssonDistanceMapImageFilter< OutputImageType, MapOutputImageType> FilterType;

    //creating the PCAShapeModelEstimator and initializing
    typedef itk::ImagePCAShapeModelEstimator<MapOutputImageType, InternalImageType> PCAEstimatorType;
    PCAEstimatorType::Pointer model = PCAEstimatorType::New();
    model->DebugOn();

    model->SetNumberOfTrainingImages(m_ui.ImageList->topLevelItemCount());
    model->SetNumberOfPrincipalComponentsRequired(NUMPC);

    //setting up and running the filters on all of the training images
    for (unsigned int k = 0; k < m_ui.ImageList->topLevelItemCount(); k++)
    {

        ThresholdingFilterType::Pointer thresholder = ThresholdingFilterType::New();

        thresholder->SetLowerThreshold(255);
        thresholder->SetUpperThreshold(255);
        thresholder->SetOutsideValue(0);
        thresholder->SetInsideValue(255);

        ReaderType::Pointer reader = ReaderType::New();
        FilterType::Pointer filter = FilterType::New();


        reader->SetFileName(m_ui.ImageList->topLevelItem(k)->text(1).toStdString().c_str());
        thresholder->SetInput(reader->GetOutput());
        filter->SetInput(thresholder->GetOutput());
        filter->Update();
        model->SetInput(k, filter->GetOutput());
    }
    model->Update();
    model->Print(std::cout);

    for (int i=0;i<NUMPC+1;i++)
    {
        QString name;
        if (i==0)
        {
            name = "MeanImage";
        }
        else
        {
            name = QString("PCAImage_%1").arg(i);
        }
        AddITKImageNode(model->GetOutput(i), name.toStdString().c_str());
    }
    

    IQF_MitkRenderWindow* pRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
    pRenderWindow->GetRenderingManager()->RequestUpdateAll();
}

template<class TImage>
void PCAShapeModelEstimatorView::AddITKImageNode(TImage* itkImage, const char* name)
{
    mitk::Image::Pointer image;
    mitk::CastToMitkImage(itkImage, image);
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    node->SetData(image);
    node->SetName(name);

    IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    pDataManager->GetDataStorage()->Add(node);
}


void PCAShapeModelEstimatorView::BrowseFile()
{
    QString dirStr = QFileDialog::getExistingDirectory(this,"Select An Directory Consist Training Image.");
    if (dirStr.isEmpty())
    {
        return;
    }
    m_ui.DirecotyLE->setText(dirStr);
    QDir dir(dirStr);
    QFileInfoList files = dir.entryInfoList(QDir::Files, QDir::Name);
    
    m_ui.ImageList->clear();

    QList<QTreeWidgetItem*> items;
    foreach (QFileInfo fileInfo , files)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(QStringList()<< fileInfo.fileName()<<fileInfo.filePath());
        items.append(item);
    }
    m_ui.ImageList->addTopLevelItems(items);

}