#include "ImageNormalizationView.h" 
#include "iqf_main.h"

#include <QFileDialog>
#include <QDebug>

#include <vtkNIFTIImageReader.h>
#include <vtkNIFTIImageWriter.h>
#include <vtkImageShiftScale.h>

#include <itkImageFileReader.h>
#include <itkNormalizeImageFilter.h>
#include <itkImageFileWriter.h>

#include <ITKImageTypeDef.h>
  
ImageNormalizationView::ImageNormalizationView() :MitkPluginView() 
{
}
 
ImageNormalizationView::~ImageNormalizationView() 
{
}
 
void ImageNormalizationView::CreateView()
{
    m_ui.setupUi(this);
    connect(m_ui.AddBtn, &QPushButton::clicked, this, &ImageNormalizationView::AddFile);
    connect(m_ui.RemoveBtn, &QPushButton::clicked, this, &ImageNormalizationView::RemoveFile);
    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &ImageNormalizationView::Apply);
} 
 
WndHandle ImageNormalizationView::GetPluginHandle() 
{
    return this; 
}

void ImageNormalizationView::Apply()
{
    QString dirStr = QFileDialog::getExistingDirectory(this, "Select An Directory To Store Results.");
    if (dirStr.isEmpty())
    {
        return;
    }
    QString fileName;
    for (int i=0;i<m_ui.FileList->count();i++)
    {
        fileName = m_ui.FileList->item(i)->text();
        NormalizeImage(fileName, dirStr);
    }
}

void ImageNormalizationView::AddFile()
{
    QStringList files = QFileDialog::getOpenFileNames();
    m_ui.FileList->addItems(files);
}

void ImageNormalizationView::RemoveFile()
{
    
}

void ImageNormalizationView::NormalizeImage(const QString& filename, const QString& outputDir)
{
      qDebug() << "Normalize Image " << filename;
      QFileInfo fi(filename);
      QString outputFileName = outputDir + "/" + fi.baseName() + ".nii";

//    auto reader = vtkSmartPointer<vtkNIFTIImageReader>::New();
//    reader->SetFileName(filename.toStdString().c_str());
//    reader->Update();
//
//    vtkSmartPointer<vtkImageShiftScale> shiftScaleFilter =
//        vtkSmartPointer<vtkImageShiftScale>::New();
//    shiftScaleFilter->SetOutputScalarTypeToUnsignedChar();
//#if VTK_MAJOR_VERSION <= 5
//    shiftScaleFilter->SetInputData(reader->GetOutput());
//#else
//    shiftScaleFilter->SetInputData(image);
//#endif
//    shiftScaleFilter->SetShift(-1.0f * reader->GetOutput()->GetScalarRange()[0]); // brings the lower bound to 0
//    float oldRange = reader->GetOutput()->GetScalarRange()[1] - reader->GetOutput()->GetScalarRange()[0];
//    float newRange = 1.0; // We want the output [0,255]
//
//    shiftScaleFilter->SetScale(newRange / oldRange);
//    shiftScaleFilter->Update();
//
//    auto writer = vtkSmartPointer<vtkNIFTIImageWriter>::New();
//    
//    writer->SetInputData(shiftScaleFilter->GetOutput());
//    writer->SetFileName(outputFileName.toStdString().c_str());
//    writer->Update();
//


    auto reader = itk::ImageFileReader<Float3DImageType>::New();
    reader->SetFileName(filename.toStdString().c_str());
    reader->Update();

    auto filter = itk::NormalizeImageFilter<Float3DImageType, Float3DImageType>::New();
    filter->SetInput(reader->GetOutput());
    filter->Update();

    auto writer = itk::ImageFileWriter<Float3DImageType>::New();
    writer->SetInput(filter->GetOutput());
    writer->SetFileName(outputFileName.toStdString().c_str());
    writer->Write();
    qDebug() << "Save file " << outputFileName;
}