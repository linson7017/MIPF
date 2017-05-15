#include "CQF_MainCommand.h"
#include "MitkMain/mitk_command_def.h"
#include <string.h>

//mitk
#include "MitkMain/IQF_MitkDataManager.h"
#include "MitkMain/IQF_MitkRenderWindow.h"
#include "MitkMain/IQF_MitkReference.h"

#include "mitkRenderingManager.h"
#include "mitkToolManager.h"
#include <mitkToolManagerProvider.h>
#include "mitkApplicationCursor.h"
#include "mitkSegmentationObjectFactory.h"
#include "mitkCameraController.h"
#include "mitkLabelSetImage.h"

#include "QmitkIOUtil.h"
#include <QFileDialog>

#include "iqf_main.h"

//qt
#include <QMessageBox>

#include "QmitkNewSegmentationDialog.h"
#include "QmitkStdMultiWidget.h"
#include "QmitkSegmentationOrganNamesHandling.cpp"

#include "usModuleResource.h"
#include "usModuleResourceStream.h"

CQF_MainCommand::CQF_MainCommand(QF::IQF_Main* pMain)
{
    m_pMain = pMain;

    mitk::NodePredicateDataType::Pointer isDwi = mitk::NodePredicateDataType::New("DiffusionImage");
    mitk::NodePredicateDataType::Pointer isDti = mitk::NodePredicateDataType::New("TensorImage");
    mitk::NodePredicateDataType::Pointer isQbi = mitk::NodePredicateDataType::New("QBallImage");
    mitk::NodePredicateOr::Pointer isDiffusionImage = mitk::NodePredicateOr::New(isDwi, isDti);
    isDiffusionImage = mitk::NodePredicateOr::New(isDiffusionImage, isQbi);
    m_IsOfTypeImagePredicate = mitk::NodePredicateOr::New(isDiffusionImage, mitk::TNodePredicateDataType<mitk::Image>::New());

    m_IsBinaryPredicate = mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true));
    m_IsNotBinaryPredicate = mitk::NodePredicateNot::New(m_IsBinaryPredicate);

    m_IsNotABinaryImagePredicate = mitk::NodePredicateAnd::New(m_IsOfTypeImagePredicate, m_IsNotBinaryPredicate);
    m_IsABinaryImagePredicate = mitk::NodePredicateAnd::New(m_IsOfTypeImagePredicate, m_IsBinaryPredicate);

    m_IsASegmentationImagePredicate = mitk::NodePredicateOr::New(m_IsABinaryImagePredicate, mitk::TNodePredicateDataType<mitk::LabelSetImage>::New());
    m_IsAPatientImagePredicate = mitk::NodePredicateAnd::New(m_IsNotABinaryImagePredicate, mitk::NodePredicateNot::New(mitk::TNodePredicateDataType<mitk::LabelSetImage>::New()));
}


CQF_MainCommand::~CQF_MainCommand()
{
}

void CQF_MainCommand::Release()
{
    delete this;
}

int CQF_MainCommand::GetCommandCount()
{
    return 3;
}

const char* CQF_MainCommand::GetCommandID(int iIndex)
{
    switch (iIndex)
    {
    case 0:
        return "MITK_MAIN_COMMAND_TOOLADD";
    case 1:
        return "MITK_MAIN_COMMAND_CREATE_NEW_SEGMENTATION";
    case 2:
        return "MITK_COMMAND_SEGMENTATION_SELECTION_CHANGED";
    default:
        return "";
        break;
    }
}

bool CQF_MainCommand::ExecuteCommand(const char* szCommandID, QF::IQF_PropertySet* pInParam, QF::IQF_PropertySet* pOutParam)
{
    if (strcmp(szCommandID, "MITK_MAIN_COMMAND_TOOLADD") == 0)
    {
        IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
        if (pMitkDataManager)
        {
            mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
            if (pMitkDataManager->GetCurrentNode().IsNotNull() && toolManager->GetWorkingData().size() != 0)
            {
                toolManager->SetReferenceData(const_cast<mitk::DataNode*>(pMitkDataManager->GetCurrentNode().GetPointer()));
                toolManager->SetWorkingData(const_cast<mitk::DataNode*>(toolManager->GetWorkingData(0)));
                toolManager->ActivateTool(1);
                return true;
            }
            else
            {
                MITK_ERROR << "'There is no working data in data tree ! Please add a new segmentation node !";
                return false;
            }

        }
        else
        {
            MITK_ERROR << "'Data manager has not been initialized !";
            return false;
        }
    }
    else if (strcmp(szCommandID, "MITK_MAIN_COMMAND_CREATE_NEW_SEGMENTATION") == 0)
    {
        CreateSegmentationNode();
        return true;
    }
    /*else if (strcmp(szCommandID, "MITK_COMMAND_SEGMENTATION_SELECTION_CHANGED") == 0)
    {
        IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)pInParam;
        OnSelectionChanged(pMitkDataManager->GetCurrentNode().GetPointer());
    }*/
    else
    {
        return false;
    }
}

mitk::DataNode* CQF_MainCommand::CreateSegmentationNode()
{
    IQF_MitkReference* pMitkReferences = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
    IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);

    mitk::DataNode::Pointer node = pMitkDataManager->GetCurrentNode();
    if (node.IsNotNull())
    {
        mitk::Image::Pointer image = dynamic_cast<mitk::Image*>(node->GetData());
        if (image.IsNotNull())
        {
            if (image->GetDimension() > 1)
            {
                // ask about the name and organ type of the new segmentation
                QmitkNewSegmentationDialog* dialog = new QmitkNewSegmentationDialog(NULL); // needs a QWidget as parent, "this" is not QWidget
                QString storedList = pMitkReferences->GetString("Organ-Color-List");
                QStringList organColors;
                if (storedList.isEmpty())
                {
                    organColors = mitk::OrganNamesHandling::GetDefaultOrganColorString();
                }
                else
                {
                    // recover string list from BlueBerry view's preferences
                    QString storedString = "";
                    MITK_DEBUG << "storedString: " << storedString.toStdString();
                    // match a string consisting of any number of repetitions of either "anything but ;" or "\;". This matches everything until the next unescaped ';'
                    QRegExp onePart("(?:[^;]|\\\\;)*");
                    MITK_DEBUG << "matching " << onePart.pattern().toStdString();
                    int count = 0;
                    int pos = 0;
                    while ((pos = onePart.indexIn(storedString, pos)) != -1)
                    {
                        ++count;
                        int length = onePart.matchedLength();
                        if (length == 0) break;
                        QString matchedString = storedString.mid(pos, length);
                        MITK_DEBUG << "   Captured length " << length << ": " << matchedString.toStdString();
                        pos += length + 1; // skip separating ';'

                                           // unescape possible occurrences of '\;' in the string
                        matchedString.replace("\\;", ";");

                        // add matched string part to output list
                        organColors << matchedString;
                    }
                    MITK_DEBUG << "Captured " << count << " organ name/colors";
                }

                dialog->SetSuggestionList(organColors);

                int dialogReturnValue = dialog->exec();

                if (dialogReturnValue == QDialog::Rejected) return NULL; // user clicked cancel or pressed Esc or something similar

                                                                    // ask the user about an organ type and name, add this information to the image's (!) propertylist
                                                                    // create a new image of the same dimensions and smallest possible pixel type
                mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
                if (toolManager->GetTools().size() == 0)
                {
                    toolManager->InitializeTools();
                }
                if (toolManager->GetDataStorage() == NULL)
                {
                    toolManager->SetDataStorage(*pMitkDataManager->GetDataStorage().GetPointer());
                }
                mitk::Tool* firstTool = toolManager->GetToolById(0);
                if (firstTool)
                {
                    try
                    {
                        std::string newNodeName = dialog->GetSegmentationName().toStdString();
                        if (newNodeName.empty())
                            newNodeName = "no_name";

                        mitk::DataNode::Pointer emptySegmentation =
                            firstTool->CreateEmptySegmentationNode(image, newNodeName, dialog->GetColor());

                        // initialize showVolume to false to prevent recalculating the volume while working on the segmentation
                        emptySegmentation->SetProperty("showVolume", mitk::BoolProperty::New(false));

                        if (!emptySegmentation) return NULL; // could be aborted by user

                        mitk::OrganNamesHandling::UpdateOrganList(organColors, dialog->GetSegmentationName(), dialog->GetColor());

                        /*
                        escape ';' here (replace by '\;'), see longer comment above
                        */
                        QString stringForStorage = organColors.replaceInStrings(";", "\\;").join(";");
                        MITK_DEBUG << "Will store: " << stringForStorage;
                        //this->GetPreferences()->Put("Organ-Color-List", stringForStorage);
                        //this->GetPreferences()->Flush();
                        pMitkReferences->SetString("Organ-Color-List", stringForStorage.toStdString().c_str());

                        if (mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0))
                        {
                            mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0)->SetSelected(false);
                        }
                        emptySegmentation->SetSelected(true);
                        pMitkDataManager->GetDataStorage()->Add(emptySegmentation, node); // add as a child, because the segmentation "derives" from the original

                        this->ApplyDisplayOptions(emptySegmentation);
                        //this->OnSelectionChanged(emptySegmentation);

                        //m_Controls->segImageSelector->SetSelectedNode(emptySegmentation);
                        mitk::RenderingManager::GetInstance()->InitializeViews(emptySegmentation->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
                        return emptySegmentation.GetPointer();
                    }
                    catch (std::bad_alloc)
                    {
                        QMessageBox::warning(NULL, QObject::tr("Create new segmentation"), QObject::tr("Could not allocate memory for new segmentation"));
                        return NULL;
                    }
                }
            }
            else
            {
                QMessageBox::information(NULL, QObject::tr("Segmentation"), QObject::tr("Segmentation is currently not supported for 2D images"));
            }
        }
    }
    else
    {
        MITK_ERROR << "'Create new segmentation' button should never be clickable unless a patient image is selected...";
        return NULL;
    }
}

void CQF_MainCommand::ApplyDisplayOptions(mitk::DataNode* node)
{
    if (!node) return;

    IQF_MitkReference* pMitkReferences = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);

    bool isBinary(false);
    node->GetPropertyValue("binary", isBinary);

    if (isBinary)
    {
        node->SetProperty("outline binary", mitk::BoolProperty::New(pMitkReferences->GetBool("draw outline")));
        node->SetProperty("outline width", mitk::FloatProperty::New(2.0));
        node->SetProperty("opacity", mitk::FloatProperty::New(pMitkReferences->GetBool("draw outline", true) ? 1.0 : 0.3));
        node->SetProperty("volumerendering", mitk::BoolProperty::New(pMitkReferences->GetBool("volume rendering", false)));
    }
}

void CQF_MainCommand::OnSelectionChanged(mitk::DataNode::Pointer node)
{
    std::vector<mitk::DataNode::Pointer> nodes;
    nodes.push_back(node);
    this->OnSelectionChanged(nodes);
}

void CQF_MainCommand::OnSelectionChanged(std::vector<mitk::DataNode::Pointer> nodes)
{
    IQF_MitkReference* pMitkReferences = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);
    IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    if (nodes.size() != 0)
    {
        std::string markerName = "Position";
        unsigned int numberOfNodes = nodes.size();
        std::string nodeName = nodes.at(0)->GetName();
        if ((numberOfNodes == 1) && (nodeName.find(markerName) == 0))
        {
            this->OnContourMarkerSelected(nodes.at(0));
            return;
        }
    }
    if (nodes.size() == 1)
    {
        mitk::DataNode::Pointer selectedNode = nodes.at(0);
        if (selectedNode.IsNull())
        {
            return;
        }

        mitk::Image::Pointer selectedImage = dynamic_cast<mitk::Image*>(selectedNode->GetData());
        if (selectedImage.IsNull())
        {
            SetToolManagerSelection(NULL, NULL);
            return;
        }
        else
        {
            bool isASegmentation(false);
            selectedNode->GetBoolProperty("binary", isASegmentation);
            mitk::LabelSetImage::Pointer labelSetImage = dynamic_cast<mitk::LabelSetImage*>(selectedNode->GetData());
            isASegmentation = isASegmentation || labelSetImage.IsNotNull();

            if (isASegmentation)
            {
                //If a segmentation is selected find a possible reference image:
                mitk::DataStorage::SetOfObjects::ConstPointer sources = pMitkDataManager->GetDataStorage()->GetSources(selectedNode,m_IsAPatientImagePredicate);
                mitk::DataNode::Pointer refNode;
                if (sources->Size() != 0)
                {
                    refNode = sources->ElementAt(0);

                    refNode->SetVisibility(true);
                    selectedNode->SetVisibility(true);
                    SetToolManagerSelection(refNode, selectedNode);

                    mitk::DataStorage::SetOfObjects::ConstPointer otherSegmentations = pMitkDataManager->GetDataStorage()->GetSubset(m_IsASegmentationImagePredicate);
                    for (mitk::DataStorage::SetOfObjects::const_iterator iter = otherSegmentations->begin(); iter != otherSegmentations->end(); ++iter)
                    {
                        mitk::DataNode* node = *iter;
                        if (dynamic_cast<mitk::Image*>(node->GetData()) != selectedImage.GetPointer())
                            node->SetVisibility(false);
                    }

                    mitk::DataStorage::SetOfObjects::ConstPointer otherPatientImages = pMitkDataManager->GetDataStorage()->GetSubset(m_IsAPatientImagePredicate);
                    for (mitk::DataStorage::SetOfObjects::const_iterator iter = otherPatientImages->begin(); iter != otherPatientImages->end(); ++iter)
                    {
                        mitk::DataNode* node = *iter;
                        if (dynamic_cast<mitk::Image*>(node->GetData()) != dynamic_cast<mitk::Image*>(refNode->GetData()))
                            node->SetVisibility(false);
                    }
                }
                else
                {
                    mitk::DataStorage::SetOfObjects::ConstPointer possiblePatientImages = pMitkDataManager->GetDataStorage()->GetSubset(m_IsAPatientImagePredicate);

                    for (mitk::DataStorage::SetOfObjects::ConstIterator it = possiblePatientImages->Begin(); it != possiblePatientImages->End(); it++)
                    {
                        refNode = it->Value();

                        if (this->CheckForSameGeometry(selectedNode, it->Value()))
                        {
                            refNode->SetVisibility(true);
                            selectedNode->SetVisibility(true);

                            mitk::DataStorage::SetOfObjects::ConstPointer otherSegmentations = pMitkDataManager->GetDataStorage()->GetSubset(m_IsASegmentationImagePredicate);
                            for (mitk::DataStorage::SetOfObjects::const_iterator iter = otherSegmentations->begin(); iter != otherSegmentations->end(); ++iter)
                            {
                                mitk::DataNode* node = *iter;
                                if (dynamic_cast<mitk::Image*>(node->GetData()) != selectedImage.GetPointer())
                                    node->SetVisibility(false);
                            }

                            mitk::DataStorage::SetOfObjects::ConstPointer otherPatientImages = pMitkDataManager->GetDataStorage()->GetSubset(m_IsAPatientImagePredicate);
                            for (mitk::DataStorage::SetOfObjects::const_iterator iter = otherPatientImages->begin(); iter != otherPatientImages->end(); ++iter)
                            {
                                mitk::DataNode* node = *iter;
                                if (dynamic_cast<mitk::Image*>(node->GetData()) != dynamic_cast<mitk::Image*>(refNode->GetData()))
                                    node->SetVisibility(false);
                            }
                            this->SetToolManagerSelection(refNode, selectedNode);

                            //Doing this we can assure that the segmenation is always visible if the segmentation and the patient image are at the
                            //same level in the datamanager
                            int layer(10);
                            refNode->GetIntProperty("layer", layer);
                            layer++;
                            selectedNode->SetProperty("layer", mitk::IntProperty::New(layer));
                            return;
                        }
                    }
                    this->SetToolManagerSelection(NULL, selectedNode);
                }
                mitk::RenderingManager::GetInstance()->InitializeViews(selectedNode->GetData()->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
            }
            else
            {
                if (mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetReferenceData(0) != selectedNode)
                {
                    SetToolManagerSelection(selectedNode, NULL);
                    //May be a bug in the selection services. A node which is deselected will be passed as selected node to the OnSelectionChanged function
                    if (!selectedNode->IsVisible(mitk::BaseRenderer::GetInstance(mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1"))))
                        selectedNode->SetVisibility(true);
                    //   this->UpdateWarningLabel(tr("The selected patient image does not\nmatchwith the selected segmentation!"));
                     //  this->SetToolSelectionBoxesEnabled(false);
                }
            }
        }


        //if (m_Controls->lblSegmentationWarnings->isVisible()) // "RenderingManagerReinitialized()" caused a warning. we do not need to go any further
        //    return;
        RenderingManagerReinitialized();
    }
}

bool CQF_MainCommand::CheckForSameGeometry(const mitk::DataNode *node1, const mitk::DataNode *node2) const
{
    bool isSameGeometry(true);

    mitk::Image* image1 = dynamic_cast<mitk::Image*>(node1->GetData());
    mitk::Image* image2 = dynamic_cast<mitk::Image*>(node2->GetData());
    if (image1 && image2)
    {
        mitk::BaseGeometry* geo1 = image1->GetGeometry();
        mitk::BaseGeometry* geo2 = image2->GetGeometry();

        isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetOrigin(), geo2->GetOrigin());
        isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetExtent(0), geo2->GetExtent(0));
        isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetExtent(1), geo2->GetExtent(1));
        isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetExtent(2), geo2->GetExtent(2));
        isSameGeometry = isSameGeometry && mitk::Equal(geo1->GetSpacing(), geo2->GetSpacing());
        isSameGeometry = isSameGeometry && mitk::MatrixEqualElementWise(geo1->GetIndexToWorldTransform()->GetMatrix(), geo2->GetIndexToWorldTransform()->GetMatrix());

        return isSameGeometry;
    }
    else
    {
        return false;
    }
}

void CQF_MainCommand::RenderingManagerReinitialized()
{
    
}

void CQF_MainCommand::OnContourMarkerSelected(const mitk::DataNode *node)
{
    IQF_MitkRenderWindow* pMitkRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
    QmitkStdMultiWidget* pMultiWidget = pMitkRenderWindow->GetMitkStdMultiWidget();
    if (!pMultiWidget)
    {
        return;
    }
    QmitkRenderWindow* selectedRenderWindow = 0;
    QmitkRenderWindow* RenderWindow1 =
        pMultiWidget->GetRenderWindow1();
    QmitkRenderWindow* RenderWindow2 =
        pMultiWidget->GetRenderWindow2();
    QmitkRenderWindow* RenderWindow3 =
        pMultiWidget->GetRenderWindow3();
    QmitkRenderWindow* RenderWindow4 =
        pMultiWidget->GetRenderWindow4();
    bool PlanarFigureInitializedWindow = false;

    // find initialized renderwindow
    if (node->GetBoolProperty("PlanarFigureInitializedWindow",
        PlanarFigureInitializedWindow, RenderWindow1->GetRenderer()))
    {
        selectedRenderWindow = RenderWindow1;
    }
    if (!selectedRenderWindow && node->GetBoolProperty(
        "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
        RenderWindow2->GetRenderer()))
    {
        selectedRenderWindow = RenderWindow2;
    }
    if (!selectedRenderWindow && node->GetBoolProperty(
        "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
        RenderWindow3->GetRenderer()))
    {
        selectedRenderWindow = RenderWindow3;
    }
    if (!selectedRenderWindow && node->GetBoolProperty(
        "PlanarFigureInitializedWindow", PlanarFigureInitializedWindow,
        RenderWindow4->GetRenderer()))
    {
        selectedRenderWindow = RenderWindow4;
    }

    // make node visible
    if (selectedRenderWindow)
    {
        std::string nodeName = node->GetName();
        unsigned int t = nodeName.find_last_of(" ");
        unsigned int id = atof(nodeName.substr(t + 1).c_str()) - 1;

        /*{
        ctkPluginContext* context = mitk::PluginActivator::getContext();
        ctkServiceReference ppmRef = context->getServiceReference<mitk::PlanePositionManagerService>();
        mitk::PlanePositionManagerService* service = context->getService<mitk::PlanePositionManagerService>(ppmRef);
        selectedRenderWindow->GetSliceNavigationController()->ExecuteOperation(service->GetPlanePosition(id));
        context->ungetService(ppmRef);
        }*/

        selectedRenderWindow->GetRenderer()->GetCameraController()->Fit();
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
}

void CQF_MainCommand::SetToolManagerSelection(const mitk::DataNode* referenceData, const mitk::DataNode* workingData)
{
    // called as a result of new BlueBerry selections
    //   tells the ToolManager for manual segmentation about new selections
    //   updates GUI information about what the user should select
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    toolManager->SetReferenceData(const_cast<mitk::DataNode*>(referenceData));
    toolManager->SetWorkingData(const_cast<mitk::DataNode*>(workingData));

}


void CQF_MainCommand::NodeAdded(const mitk::DataNode *node)
{
    bool isBinary(false);
    bool isHelperObject(false);
    bool isImage(false);
    node->GetBoolProperty("binary", isBinary);
    mitk::LabelSetImage::Pointer labelSetImage = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
    isBinary = isBinary || labelSetImage.IsNotNull();
    node->GetBoolProperty("helper object", isHelperObject);

    if (dynamic_cast<mitk::Image*>(node->GetData()))
    {
        isImage = true;
    }

    if (m_AutoSelectionEnabled)
    {
        if (!isBinary && isImage)
        {
            //FireNodeSelected(const_cast<mitk::DataNode*>(node));
        }
    }

    if (isImage && !isHelperObject)
    {
        itk::SimpleMemberCommand<MitkSegmentation>::Pointer command = itk::SimpleMemberCommand<MitkSegmentation>::New();
        command->SetCallbackFunction(this, &MitkSegmentation::OnWorkingNodeVisibilityChanged);
        m_WorkingDataObserverTags.insert(std::pair<mitk::DataNode*, unsigned long>(const_cast<mitk::DataNode*>(node), node->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), command)));

        itk::SimpleMemberCommand<MitkSegmentation>::Pointer command2 = itk::SimpleMemberCommand<MitkSegmentation>::New();
        command2->SetCallbackFunction(this, &MitkSegmentation::OnBinaryPropertyChanged);
        m_BinaryPropertyObserverTags.insert(std::pair<mitk::DataNode*, unsigned long>(const_cast<mitk::DataNode*>(node), node->GetProperty("binary")->AddObserver(itk::ModifiedEvent(), command2)));

        this->ApplyDisplayOptions(const_cast<mitk::DataNode*>(node));
        m_Controls->segImageSelector->setCurrentIndex(m_Controls->segImageSelector->Find(node));
    }
}

