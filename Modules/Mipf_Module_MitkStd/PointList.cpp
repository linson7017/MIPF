#include "PointList.h"
#include <mitkPointSetDataInteractor.h>

#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <mitkIOUtil.h>
#include "MitkMain/IQF_MitkDataManager.h"
#include "iqf_main.h"

IQF_MitkPointList* PointListFactory::CreatePointList()
{
    return new PointList(m_pMain);
}

PointList::PointList(QF::IQF_Main* pMain):m_pMain(pMain), m_PointSetNode(NULL), m_bAddingPoint(false)
{
    m_pSubject = QF::QF_CreateSubjectObject();
}

PointList::~PointList()
{
    delete m_pSubject;
}

void PointList::Release()
{
    delete this;
}

void PointList::Attach(QF::IQF_Observer* observer)
{
    m_pSubject->Attach(observer);
    
}

void PointList::Detach(QF::IQF_Observer* observer)
{
    m_pSubject->Detach(observer);
}

void PointList::Initialize()
{
    AddPoint(false);
    m_DataInteractor = NULL;
    m_PointSetNode = NULL;
}

void PointList::CreateNewPointSetNode(mitk::DataNode * pointSetNode, bool bDirectUse)
{
    mitk::PointSet::Pointer pointSet = mitk::PointSet::New();
    pointSetNode->SetData(pointSet);
    pointSetNode->SetName("seed points for tracking");
    pointSetNode->SetProperty("helper object", mitk::BoolProperty::New(true));
    pointSetNode->SetProperty("layer", mitk::IntProperty::New(1024));

    if (bDirectUse)
    {
        IQF_MitkDataManager* pDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        if (pDataManager)
        {
            pDataManager->GetDataStorage()->Add(pointSetNode);
            SetPointSetNode(pointSetNode);
        }
    }
}

void PointList::AddPoint(bool bAdd)
{
    if (m_PointSetNode.IsNotNull())
    {
        if (bAdd)
        {
            m_DataInteractor = m_PointSetNode->GetDataInteractor();
            // If no data Interactor is present create a new one
            if (m_DataInteractor.IsNull())
            {
                // Create PointSetData Interactor
                m_DataInteractor = mitk::PointSetDataInteractor::New();
                // Load the according state machine for regular point set interaction
                m_DataInteractor->LoadStateMachine("PointSet.xml");
                // Set the configuration file that defines the triggers for the transitions
                m_DataInteractor->SetEventConfig("PointSetConfig.xml");
                // set the DataNode (which already is added to the DataStorage
                m_DataInteractor->SetDataNode(m_PointSetNode);
                m_bAddingPoint = true;
            }
        }
        else
        {
            m_PointSetNode->SetDataInteractor(NULL);
            m_DataInteractor = NULL;
            m_bAddingPoint = false;
        }
        
    }
}

bool PointList::InsertPoint(const double x, const double y, const double z)
{
    if (m_PointSetNode.IsNotNull())
    {
        mitk::PointSet* pointSet = dynamic_cast<mitk::PointSet*>(m_PointSetNode->GetData());
        if (pointSet)
        {
            mitk::Point3D point;
            point[0] = x;
            point[1] = y;
            point[2] = z;
            pointSet->InsertPoint(point);
            m_PointSetNode->Modified();
        }
        else
        {
            return false;
        }       
    }
    else
    {
        return false;
    }
}

void PointList::SetPointSetNode(mitk::DataNode *newNode)
{
    m_PointSetNode = newNode;
    if (m_DataInteractor.IsNotNull())
        m_DataInteractor->SetDataNode(newNode);

    mitk::PointSet::Pointer pointSet = this->CheckForPointSetInNode(m_PointSetNode);
    if (pointSet.IsNotNull())
    {
        // add new observer for modified if necessary
        itk::ReceptorMemberCommand<PointList>::Pointer modCommand =
            itk::ReceptorMemberCommand<PointList>::New();
        modCommand->SetCallbackFunction(this, &PointList::OnPointSetChanged);
        m_PointSetModifiedObserverTag = pointSet->AddObserver(itk::ModifiedEvent(), modCommand);

        // add new observer for detele if necessary
        itk::ReceptorMemberCommand<PointList>::Pointer delCommand =
            itk::ReceptorMemberCommand<PointList>::New();
        delCommand->SetCallbackFunction(this, &PointList::OnPointSetDeleted);
        m_PointSetDeletedObserverTag = pointSet->AddObserver(itk::DeleteEvent(), delCommand);
    }
    else
    {
        m_PointSetModifiedObserverTag = 0;
        m_PointSetDeletedObserverTag = 0;
    }
}

void PointList::OnPointSetChanged(const itk::EventObject &e)
{
    mitk::PointSet::Pointer ps = CheckForPointSetInNode(m_PointSetNode);
    if (ps)
    {
        int selectedIndex = ps->SearchSelectedPoint();
        m_pSubject->Notify(MITK_MESSAGE_POINTLIST_CHANGED, selectedIndex, ps.GetPointer());
    }  
}

void PointList::OnPointSetDeleted(const itk::EventObject &e)
{
    mitk::PointSet::Pointer ps = CheckForPointSetInNode(m_PointSetNode);
    if (ps)
    {
        ps->RemoveObserver(m_PointSetModifiedObserverTag);
        ps->RemoveObserver(m_PointSetDeletedObserverTag);
    }
    m_PointSetModifiedObserverTag = 0;
    m_PointSetDeletedObserverTag = 0;

    m_pSubject->Notify(MITK_MESSAGE_POINTLIST_REMOVED, 0, ps);
}

mitk::PointSet *PointList::CheckForPointSetInNode(mitk::DataNode *node) const
{
    if (node != NULL)
    {
        mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet *>(node->GetData());
        if (pointSet.IsNotNull())
            return pointSet;
    }
    return NULL;
}

mitk::DataNode *PointList::GetPointSetNode()
{
    return m_PointSetNode;
}

mitk::PointSet* PointList::GetPointSet()
{
    return dynamic_cast<mitk::PointSet *>(m_PointSetNode->GetData());
}

void PointList::SavePoints()
{
    if ((dynamic_cast<mitk::PointSet *>(m_PointSetNode->GetData())) == NULL)
        return; // don't write empty point sets. If application logic requires something else then do something else.
    if ((dynamic_cast<mitk::PointSet *>(m_PointSetNode->GetData()))->GetSize() == 0)
        return;

    // take the previously defined name of node as proposal for filename
    std::string nodeName = m_PointSetNode->GetName();
    nodeName = "/" + nodeName + ".mps";
    QString fileNameProposal = QString();
    fileNameProposal.append(nodeName.c_str());

    QString aFilename = QFileDialog::getSaveFileName(
        NULL, "Save point set", QDir::currentPath() + fileNameProposal, "MITK Pointset (*.mps)");
    if (aFilename.isEmpty())
        return;

    try
    {
        mitk::IOUtil::Save(m_PointSetNode->GetData(), aFilename.toStdString());
    }
    catch (...)
    {
        QMessageBox::warning(NULL,
            "Save point set",
            QString("File writer reported problems writing %1\n\n"
                "PLEASE CHECK output file!")
            .arg(aFilename));
    }
}

mitk::PointSet::Pointer PointList::LoadPoints()
{
    // get the name of the file to load
    QString filename = QFileDialog::getOpenFileName(NULL, "Open MITK Pointset", "", "MITK Point Sets (*.mps)");
    if (filename.isEmpty())
        return NULL;

    // attempt to load file
    try
    {
        mitk::PointSet::Pointer pointSet = mitk::IOUtil::LoadPointSet(filename.toStdString());
        if (pointSet.IsNull())
        {
            QMessageBox::warning(NULL, "Load point set", QString("File reader could not read %1").arg(filename));
            return NULL;
        }

        // loading successful
        /*m_PointSetNode->SetData(pointSet);
        SetPointSetNode(m_PointSetNode);
        AddPoint(m_bAddingPoint);*/
        return pointSet;
    }
    catch (...)
    {
        QMessageBox::warning(NULL, "Load point set", QString("File reader collapsed while reading %1").arg(filename));
        return NULL;
    }
    //emit PointListChanged();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}