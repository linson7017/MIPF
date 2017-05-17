#include "CMitkSegmentation.h"
#include "iqf_main.h"
#include "Res/R.h"

#include "MitkMain/mitk_main_msg.h"
//qt
#include <QMessageBox>

//qmitk
#include "QmitkStdMultiWidget.h"
#include "QmitkNewSegmentationDialog.h"
#include "QmitkToolSelectionBox.h"
//mitk
#include <mitkSurfaceToImageFilter.h>
#include <mitkToolManagerProvider.h>
#include "mitkProperties.h"
#include "mitkSegTool2D.h"
#include "mitkStatusBar.h"

#include "mitkVtkResliceInterpolationProperty.h"

#include "mitkApplicationCursor.h"
#include "MitkSegmentationObjectFactory.h"
#include "mitkCameraController.h"
#include "mitkLabelSetImage.h"

#include "QmitkSlicesInterpolator.h"
#include "QMitkSegmentationOrganNamesHandling.cpp"

#include "usModuleResource.h"
#include "usModuleResourceStream.h"

#include <MitkMain/IQF_MitkDataManager.h>
#include <MitkMain/IQF_MitkRenderWindow.h>
#include <MitkMain/IQF_MitkReference.h>


CMitkSegmentation::CMitkSegmentation(QF::IQF_Main* pMain):m_pMain(pMain),
m_MouseCursorSet(false), refNode(NULL),workingNode(NULL), m_inited(false)
{
    m_pMain->Attach(this);

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


    m_pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    m_pMitkRenderWindow = (IQF_MitkRenderWindow*)m_pMain->GetInterfacePtr(QF_MitkMain_RenderWindow);
    m_pMitkReferences = (IQF_MitkReference*)m_pMain->GetInterfacePtr(QF_MitkMain_Reference);


    //init tool manager
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    assert(toolManager);

    toolManager->SetDataStorage(*(m_pMitkDataManager->GetDataStorage()));
    toolManager->InitializeTools();

    toolManager->NewNodeObjectsGenerated +=
        mitk::MessageDelegate1<CMitkSegmentation, mitk::ToolManager::DataVectorType*>(this, &CMitkSegmentation::NewNodeObjectsGenerated);

    m_interpolator = new QmitkSlicesInterpolator();
    m_ManualToolSelectionBox = new QmitkToolSelectionBox();
    m_ManualToolSelectionBox->SetGenerateAccelerators(true);
    m_ManualToolSelectionBox->SetDisplayedToolGroups(QObject::tr("Add Subtract Correction Paint Wipe 'Region Growing' Fill Erase 'Live Wire' '2D Fast Marching'").toStdString());
    m_ManualToolSelectionBox->SetLayoutColumns(3);
    m_ManualToolSelectionBox->SetEnabledMode(QmitkToolSelectionBox::EnabledWithReferenceAndWorkingDataVisible);
    R::Instance()->registerCustomWidget("ManualToolSelectionBox", m_ManualToolSelectionBox);
    R::Instance()->registerCustomWidget("SlicesInterpolator", m_interpolator);
}

CMitkSegmentation::~CMitkSegmentation()
{
}

void CMitkSegmentation::Update(const char* szMessage, int iValue, void* pValue) 
{
    if (strcmp(szMessage, "") == 0)
    {
       
    }
}

void CMitkSegmentation::Init()
{
    if (m_inited)
    {
        return;
    }
    mitk::DataStorage::SetOfObjects::ConstPointer segmentations = m_pMitkDataManager->GetDataStorage()->GetSubset(m_IsABinaryImagePredicate);
    mitk::DataStorage::SetOfObjects::ConstPointer image = m_pMitkDataManager->GetDataStorage()->GetSubset(m_IsAPatientImagePredicate);
    if (!image->empty()) {
        OnSelectionChanged(*image->begin());
    }

    for (mitk::DataStorage::SetOfObjects::const_iterator iter = segmentations->begin();
        iter != segmentations->end();
        ++iter)
    {
        mitk::DataNode* node = *iter;
        itk::SimpleMemberCommand<CMitkSegmentation>::Pointer command = itk::SimpleMemberCommand<CMitkSegmentation>::New();
        command->SetCallbackFunction(this, &CMitkSegmentation::OnWorkingNodeVisibilityChanged);
        m_WorkingDataObserverTags.insert(std::pair<mitk::DataNode*, unsigned long>(node, node->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), command)));

        itk::SimpleMemberCommand<CMitkSegmentation>::Pointer command2 = itk::SimpleMemberCommand<CMitkSegmentation>::New();
        command2->SetCallbackFunction(this, &CMitkSegmentation::OnBinaryPropertyChanged);
        m_BinaryPropertyObserverTags.insert(std::pair<mitk::DataNode*, unsigned long>(node, node->GetProperty("binary")->AddObserver(itk::ModifiedEvent(), command2)));
    }

    itk::SimpleMemberCommand<CMitkSegmentation>::Pointer command3 = itk::SimpleMemberCommand<CMitkSegmentation>::New();
    command3->SetCallbackFunction(this, &CMitkSegmentation::RenderingManagerReinitialized);
    m_RenderingManagerObserverTag = mitk::RenderingManager::GetInstance()->AddObserver(mitk::RenderingManagerViewsInitializedEvent(), command3);

    //this->SetToolManagerSelection(m_Controls->patImageSelector->GetSelectedNode(), m_Controls->segImageSelector->GetSelectedNode());
    
    //connect(m_ManualToolSelectionBox, SIGNAL(ToolSelected(int)), this, SLOT(OnManualTool2DSelected(int)));

    //m_interpolator->Enable3DInterpolation(true);

    SetMultiWidget(m_pMitkRenderWindow->GetMitkStdMultiWidget());
    m_inited = true;
}

void CMitkSegmentation::SetMultiWidget(QmitkStdMultiWidget* multiWidget)
{
    // save the current multiwidget as the working widget
    m_MultiWidget = multiWidget;

    // tell the interpolation about toolmanager and multiwidget (and data storage)
    if (m_MultiWidget)
    {
        mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
        m_interpolator->SetDataStorage(m_pMitkDataManager->GetDataStorage());
        QList<mitk::SliceNavigationController*> controllers;
        controllers.push_back(m_MultiWidget->GetRenderWindow1()->GetSliceNavigationController());
        controllers.push_back(m_MultiWidget->GetRenderWindow2()->GetSliceNavigationController());
        controllers.push_back(m_MultiWidget->GetRenderWindow3()->GetSliceNavigationController());
        m_interpolator->Initialize(toolManager, controllers);
    }
}


void CMitkSegmentation::NewNodeObjectsGenerated(mitk::ToolManager::DataVectorType* nodes)
{
    if (!nodes) return;

    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    if (!toolManager) return;
    for (mitk::ToolManager::DataVectorType::iterator iter = nodes->begin(); iter != nodes->end(); ++iter)
    {
        //this->FireNodeSelected(*iter);
        // only last iteration meaningful, multiple generated objects are not taken into account here
    }
}

void CMitkSegmentation::SetWorkingNode(mitk::DataNode* node)
{
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    toolManager->SetWorkingData(const_cast<mitk::DataNode*>(node));
    workingNode = node;
}
void CMitkSegmentation::SetReferenceNode(mitk::DataNode* node)
{
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    toolManager->SetReferenceData(const_cast<mitk::DataNode*>(node));
    refNode = node;
}

bool CMitkSegmentation::SelectTool(int id)
{
    IQF_MitkDataManager* pMitkDataManager = (IQF_MitkDataManager*)m_pMain->GetInterfacePtr(QF_MitkMain_DataManager);
    if (pMitkDataManager)
    {
        mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
        if (refNode && workingNode)
        {
            toolManager->SetReferenceData(refNode);
            toolManager->SetWorkingData(workingNode);
            toolManager->ActivateTool(id);
            OnManualTool2DSelected(id);
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

void CMitkSegmentation::OnManualTool2DSelected(int id)
{
    if (id >= 0)
    {
        std::string text = "Active Tool: \"";
        mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
        text += toolManager->GetToolById(id)->GetName();
        text += "\"";
        mitk::StatusBar::GetInstance()->DisplayText(text.c_str());

        us::ModuleResource resource = toolManager->GetToolById(id)->GetCursorIconResource();
        this->SetMouseCursor(resource, 0, 0);
    }
    else
    {
        this->ResetMouseCursor();
        mitk::StatusBar::GetInstance()->DisplayText("");
    }
}

void CMitkSegmentation::OnWorkingNodeVisibilityChanged()
{
    mitk::DataNode* selectedNode = m_pMitkDataManager->GetCurrentNode();
    if (!selectedNode)
    {
        this->SetToolSelectionBoxesEnabled(false);
        return;
    }

    bool selectedNodeIsVisible = selectedNode->IsVisible(mitk::BaseRenderer::GetInstance(
        mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget1")));

    if (!selectedNodeIsVisible)
    {
        this->SetToolSelectionBoxesEnabled(false);
        this->UpdateWarningLabel(QObject::tr("The selected segmentation is currently not visible!"));
    }
    else
    {
        this->SetToolSelectionBoxesEnabled(true);
        this->UpdateWarningLabel("");
    }
}

void CMitkSegmentation::OnBinaryPropertyChanged()
{
    mitk::DataStorage::SetOfObjects::ConstPointer patImages = m_pMitkDataManager->GetDataStorage()->GetSubset(m_IsAPatientImagePredicate);

    bool isBinary(false);

    for (mitk::DataStorage::SetOfObjects::ConstIterator it = patImages->Begin(); it != patImages->End(); ++it)
    {
        const mitk::DataNode* node = it->Value();
        node->GetBoolProperty("binary", isBinary);
        mitk::LabelSetImage::Pointer labelSetImage = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
        isBinary = isBinary || labelSetImage.IsNotNull();

        if (isBinary)
        {
            //m_Controls->patImageSelector->RemoveNode(node);
          //  m_Controls->segImageSelector->AddNode(node);
            this->SetToolManagerSelection(NULL, NULL);
            return;
        }
    }

    mitk::DataStorage::SetOfObjects::ConstPointer segImages = m_pMitkDataManager->GetDataStorage()->GetSubset(m_IsAPatientImagePredicate);

    isBinary = true;

    for (mitk::DataStorage::SetOfObjects::ConstIterator it = segImages->Begin(); it != segImages->End(); ++it)
    {
        const mitk::DataNode* node = it->Value();
        node->GetBoolProperty("binary", isBinary);
        mitk::LabelSetImage::Pointer labelSetImage = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
        isBinary = isBinary || labelSetImage.IsNotNull();

        if (!isBinary)
        {
           // m_Controls->segImageSelector->RemoveNode(node);
           // m_Controls->patImageSelector->AddNode(node);
            if (mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0) == node)
                mitk::ToolManagerProvider::GetInstance()->GetToolManager()->SetWorkingData(NULL);
            return;
        }
    }
}

void CMitkSegmentation::OnShowMarkerNodes(bool state)
{
    mitk::SegTool2D::Pointer manualSegmentationTool;

    unsigned int numberOfExistingTools = mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetTools().size();

    for (unsigned int i = 0; i < numberOfExistingTools; i++)
    {
        manualSegmentationTool = dynamic_cast<mitk::SegTool2D*>(mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetToolById(i));

        if (manualSegmentationTool)
        {
            if (state == true)
            {
                manualSegmentationTool->SetShowMarkerNodes(true);
            }
            else
            {
                manualSegmentationTool->SetShowMarkerNodes(false);
            }
        }
    }
}

void CMitkSegmentation::ResetMouseCursor()
{
    if (m_MouseCursorSet)
    {
        mitk::ApplicationCursor::GetInstance()->PopCursor();
        m_MouseCursorSet = false;
    }
}

void CMitkSegmentation::SetMouseCursor(const us::ModuleResource& resource, int hotspotX, int hotspotY)
{
    if (!resource) return;

    // Remove previously set mouse cursor
    if (m_MouseCursorSet)
    {
        mitk::ApplicationCursor::GetInstance()->PopCursor();
    }

    us::ModuleResourceStream cursor(resource, std::ios::binary);
    mitk::ApplicationCursor::GetInstance()->PushCursor(cursor, hotspotX, hotspotY);
    m_MouseCursorSet = true;
}

mitk::DataNode* CMitkSegmentation::CreateSegmentationNode()
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

void CMitkSegmentation::OnSelectionChanged(mitk::DataNode::Pointer node)
{
    std::vector<mitk::DataNode::Pointer> nodes;
    nodes.push_back(node);
    this->OnSelectionChanged(nodes);
}

void CMitkSegmentation::OnSelectionChanged(std::vector<mitk::DataNode::Pointer> nodes)
{
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
    if (0/*m_AutoSelectionEnabled && this->IsActivated()*/)
    {
        if (nodes.size() == 0 && refNode == NULL)
        {
            SetToolManagerSelection(NULL, NULL);
        }
        else if (nodes.size() == 1)
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
                    mitk::DataStorage::SetOfObjects::ConstPointer sources = m_pMitkDataManager->GetDataStorage()->GetSources(selectedNode, m_IsAPatientImagePredicate);
                    mitk::DataNode::Pointer refNode;
                    if (sources->Size() != 0)
                    {
                        refNode = sources->ElementAt(0);

                        refNode->SetVisibility(true);
                        selectedNode->SetVisibility(true);
                        SetToolManagerSelection(refNode, selectedNode);

                        mitk::DataStorage::SetOfObjects::ConstPointer otherSegmentations = m_pMitkDataManager->GetDataStorage()->GetSubset(m_IsASegmentationImagePredicate);
                        for (mitk::DataStorage::SetOfObjects::const_iterator iter = otherSegmentations->begin(); iter != otherSegmentations->end(); ++iter)
                        {
                            mitk::DataNode* node = *iter;
                            if (dynamic_cast<mitk::Image*>(node->GetData()) != selectedImage.GetPointer())
                                node->SetVisibility(false);
                        }

                        mitk::DataStorage::SetOfObjects::ConstPointer otherPatientImages = m_pMitkDataManager->GetDataStorage()->GetSubset(m_IsAPatientImagePredicate);
                        for (mitk::DataStorage::SetOfObjects::const_iterator iter = otherPatientImages->begin(); iter != otherPatientImages->end(); ++iter)
                        {
                            mitk::DataNode* node = *iter;
                            if (dynamic_cast<mitk::Image*>(node->GetData()) != dynamic_cast<mitk::Image*>(refNode->GetData()))
                                node->SetVisibility(false);
                        }
                    }
                    else
                    {
                        mitk::DataStorage::SetOfObjects::ConstPointer possiblePatientImages = m_pMitkDataManager->GetDataStorage()->GetSubset(m_IsAPatientImagePredicate);

                        for (mitk::DataStorage::SetOfObjects::ConstIterator it = possiblePatientImages->Begin(); it != possiblePatientImages->End(); it++)
                        {
                            refNode = it->Value();

                            if (this->CheckForSameGeometry(selectedNode, it->Value()))
                            {
                                refNode->SetVisibility(true);
                                selectedNode->SetVisibility(true);

                                mitk::DataStorage::SetOfObjects::ConstPointer otherSegmentations = m_pMitkDataManager->GetDataStorage()->GetSubset(m_IsASegmentationImagePredicate);
                                for (mitk::DataStorage::SetOfObjects::const_iterator iter = otherSegmentations->begin(); iter != otherSegmentations->end(); ++iter)
                                {
                                    mitk::DataNode* node = *iter;
                                    if (dynamic_cast<mitk::Image*>(node->GetData()) != selectedImage.GetPointer())
                                        node->SetVisibility(false);
                                }

                                mitk::DataStorage::SetOfObjects::ConstPointer otherPatientImages = m_pMitkDataManager->GetDataStorage()->GetSubset(m_IsAPatientImagePredicate);
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
                        this->UpdateWarningLabel(QObject::tr("The selected patient image does not\nmatchwith the selected segmentation!"));
                        this->SetToolSelectionBoxesEnabled(false);
                    }
                }
            }
        }

        RenderingManagerReinitialized();
    }
}

bool CMitkSegmentation::CheckForSameGeometry(const mitk::DataNode *node1, const mitk::DataNode *node2) const
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


void CMitkSegmentation::OnContourMarkerSelected(const mitk::DataNode *node)
{
    
    QmitkRenderWindow* selectedRenderWindow = 0;
    QmitkRenderWindow* RenderWindow1 =
        m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow1();
    QmitkRenderWindow* RenderWindow2 =
        m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow2();
    QmitkRenderWindow* RenderWindow3 =
        m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow3();
    QmitkRenderWindow* RenderWindow4 =
        m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow4();
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

void CMitkSegmentation::SetToolManagerSelection(const mitk::DataNode* referenceData, const mitk::DataNode* workingData)
{
    mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
    toolManager->SetReferenceData(const_cast<mitk::DataNode*>(referenceData));
    toolManager->SetWorkingData(const_cast<mitk::DataNode*>(workingData));
}

void CMitkSegmentation::RenderingManagerReinitialized()
{
    if (!m_pMitkRenderWindow->GetMitkStdMultiWidget()) { return; }

    const mitk::BaseGeometry* worldGeo = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow4()->GetSliceNavigationController()->GetCurrentGeometry3D();

    if (workingNode && worldGeo)
    {

        const mitk::BaseGeometry* workingNodeGeo = workingNode->GetData()->GetGeometry();
        const mitk::BaseGeometry* worldGeo = m_pMitkRenderWindow->GetMitkStdMultiWidget()->GetRenderWindow4()->GetSliceNavigationController()->GetCurrentGeometry3D();

        if (mitk::Equal(*workingNodeGeo->GetBoundingBox(), *worldGeo->GetBoundingBox(), mitk::eps, true))
        {
            this->SetToolManagerSelection(refNode, workingNode);
            this->SetToolSelectionBoxesEnabled(true);
            this->UpdateWarningLabel("");
        }
        else
        {
            this->SetToolManagerSelection(refNode, NULL);
            this->SetToolSelectionBoxesEnabled(false);
            this->UpdateWarningLabel(QObject::tr("Please perform a reinit on the segmentation image!"));
        }
    }
}

void CMitkSegmentation::UpdateWarningLabel(QString text)
{
    m_warningLabel = text;
}

void CMitkSegmentation::NodeAdded(const mitk::DataNode *node)
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

    if (0)
    {
        if (!isBinary && isImage)
        {
            //FireNodeSelected(const_cast<mitk::DataNode*>(node));
        }
    }

    if (isImage && !isHelperObject)
    {
        itk::SimpleMemberCommand<CMitkSegmentation>::Pointer command = itk::SimpleMemberCommand<CMitkSegmentation>::New();
        command->SetCallbackFunction(this, &CMitkSegmentation::OnWorkingNodeVisibilityChanged);
        m_WorkingDataObserverTags.insert(std::pair<mitk::DataNode*, unsigned long>(const_cast<mitk::DataNode*>(node), node->GetProperty("visible")->AddObserver(itk::ModifiedEvent(), command)));

        itk::SimpleMemberCommand<CMitkSegmentation>::Pointer command2 = itk::SimpleMemberCommand<CMitkSegmentation>::New();
        command2->SetCallbackFunction(this, &CMitkSegmentation::OnBinaryPropertyChanged);
        m_BinaryPropertyObserverTags.insert(std::pair<mitk::DataNode*, unsigned long>(const_cast<mitk::DataNode*>(node), node->GetProperty("binary")->AddObserver(itk::ModifiedEvent(), command2)));

        this->ApplyDisplayOptions(const_cast<mitk::DataNode*>(node));
       // m_Controls->segImageSelector->setCurrentIndex(m_Controls->segImageSelector->Find(node));
    }
}

void CMitkSegmentation::NodeRemoved(const mitk::DataNode* node)
{
    bool isSeg(false);
    bool isHelperObject(false);
    node->GetBoolProperty("helper object", isHelperObject);
    node->GetBoolProperty("binary", isSeg);
    mitk::LabelSetImage::Pointer labelSetImage = dynamic_cast<mitk::LabelSetImage*>(node->GetData());
    isSeg = isSeg || labelSetImage.IsNotNull();

    mitk::Image* image = dynamic_cast<mitk::Image*>(node->GetData());
    if (isSeg && !isHelperObject && image)
    {
        //First of all remove all possible contour markers of the segmentation
        mitk::DataStorage::SetOfObjects::ConstPointer allContourMarkers = m_pMitkDataManager->GetDataStorage()->GetDerivations(node, mitk::NodePredicateProperty::New("isContourMarker"
            , mitk::BoolProperty::New(true)));

        /*ctkPluginContext* context = mitk::PluginActivator::getContext();
        ctkServiceReference ppmRef = context->getServiceReference<mitk::PlanePositionManagerService>();
        mitk::PlanePositionManagerService* service = context->getService<mitk::PlanePositionManagerService>(ppmRef);*/

        for (mitk::DataStorage::SetOfObjects::ConstIterator it = allContourMarkers->Begin(); it != allContourMarkers->End(); ++it)
        {
            std::string nodeName = node->GetName();
            unsigned int t = nodeName.find_last_of(" ");
            unsigned int id = atof(nodeName.substr(t + 1).c_str()) - 1;

            //service->RemovePlanePosition(id);

            m_pMitkDataManager->GetDataStorage()->Remove(it->Value());
        }

        //context->ungetService(ppmRef);
        //service = NULL;

        if ((mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetWorkingData(0) == node) && workingNode)
        {
            this->SetToolManagerSelection(mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetReferenceData(0), NULL);
            this->UpdateWarningLabel(QObject::tr("Select or create a segmentation"));
        }

        mitk::SurfaceInterpolationController::GetInstance()->RemoveInterpolationSession(image);
    }
    mitk::DataNode* tempNode = const_cast<mitk::DataNode*>(node);
    //Since the binary property could be changed during runtime by the user
    if (image && !isHelperObject)
    {
        node->GetProperty("visible")->RemoveObserver(m_WorkingDataObserverTags[tempNode]);
        m_WorkingDataObserverTags.erase(tempNode);
        node->GetProperty("binary")->RemoveObserver(m_BinaryPropertyObserverTags[tempNode]);
        m_BinaryPropertyObserverTags.erase(tempNode);
    }

    if ((mitk::ToolManagerProvider::GetInstance()->GetToolManager()->GetReferenceData(0) == node))
    {
        //as we don't know which node was actually removed e.g. our reference node, disable 'New Segmentation' button.
        //consider the case that there is no more image in the datastorage
        this->SetToolManagerSelection(NULL, NULL);
        this->SetToolSelectionBoxesEnabled(false);
    }
}