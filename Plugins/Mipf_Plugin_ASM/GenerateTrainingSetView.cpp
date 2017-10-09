#include "GenerateTrainingSetView.h"

//qt
#include <QFileDialog>

//qf
#include "iqf_main.h"

//mipf
#include "MitkMain/IQF_MitkIO.h"
#include "MitkMain/IQF_MitkDataManager.h"
#include "ITKImageTypeDef.h"

//itk
#include <itkImage.h>

//mitk
#include "mitkImage.h"
#include "mitkImageCast.h"
#include "mitkDataNode.h"
#include "mitkRenderingManager.h"

#include "ITK_Helpers.h"

GenerateTrainingSetView::GenerateTrainingSetView(QF::IQF_Main* pMain, QWidget* parent) :QWidget(parent), m_pMain(pMain)
{
    m_ui.setupUi(this);

    m_ui.ImageList->setHeaderLabels(QStringList() << "Name" << "Path");
    connect(m_ui.DataDirBrowseBtn, &QPushButton::clicked, this, &GenerateTrainingSetView::DataDirBrowseFile);
    connect(m_ui.OutputDirBrowseBtn, &QPushButton::clicked, this, &GenerateTrainingSetView::OutputDirBrowseFile);
    connect(m_ui.ApplyBtn, &QPushButton::clicked, this, &GenerateTrainingSetView::Apply);
}


GenerateTrainingSetView::~GenerateTrainingSetView()
{
}

void GenerateTrainingSetView::DataDirBrowseFile()
{
    QString dirStr = QFileDialog::getExistingDirectory(this, "Select An Directory.");
    if (dirStr.isEmpty())
    {
        return;
    }
    m_ui.DataDirLE->setText(dirStr);
    QDir dir(dirStr);
    QFileInfoList files = dir.entryInfoList(QDir::Files, QDir::Name);

    m_ui.ImageList->clear();

    QList<QTreeWidgetItem*> items;
    foreach(QFileInfo fileInfo, files)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << fileInfo.fileName() << fileInfo.filePath());
        items.append(item);
    }
    m_ui.ImageList->addTopLevelItems(items);
}

void GenerateTrainingSetView::OutputDirBrowseFile()
{
    QString dirStr = QFileDialog::getExistingDirectory(this, "Select An Directory.");
    if (dirStr.isEmpty())
    {
        return;
    }
    m_ui.OutputDirLE->setText(dirStr);
}

template <class TImageType>
void CreateBaseImage(TImageType* input, TImageType*output)
{

}

void GenerateTrainingSetView::Apply()
{    

    IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    IQF_MitkIO* pIO = (IQF_MitkIO*)m_pMain->GetInterfacePtr(QF_MitkMain_IO);

    for (int i=0;i<m_ui.ImageList->topLevelItemCount();i++)
    {
        QString resultName = QString("Result%1").arg(i);
        mitk::DataNode* node = pIO->Load(m_ui.ImageList->topLevelItem(i)->text(1).toStdString().c_str());
        if (!node)
        {
            return;
        }
        mitk::Image* mitkImage = dynamic_cast<mitk::Image*>(node->GetData());
        UChar3DImageType::Pointer itkImage;
        mitk::CastToItkImage(mitkImage, itkImage);

        UChar3DImageType::Pointer output = UChar3DImageType::New();

        int size[3] = { 128,128,128 };
        ITKHelpers::ExtractCentroidImageWithGivenSize(itkImage.GetPointer(), output.GetPointer(), size);

        mitk::Image::Pointer image;
        mitk::CastToMitkImage(output, image);
        mitk::DataNode::Pointer on = mitk::DataNode::New();
        on->SetData(image);
        on->SetName(resultName.toStdString().c_str());

        pDataManager->GetDataStorage()->Add(on);
        pDataManager->GetDataStorage()->Remove(node);
    }
}