#include "CutImplementation.h"

#include "mitkImage.h"
#include "mitkSurface.h"
#include "mitkRenderingManager.h"


#include "vtkPolyData.h"
#include "vtkImageData.h"

CutImplementation::CutImplementation() :m_pDataNode(nullptr), InsideOut(false) 
{

}

void CutImplementation::Release()
{
    ClearUndo();
    ClearRedo();
    delete this;
}

void CutImplementation::Cut(vtkObject* pCutData, mitk::InteractionEvent * interactionEvent)
{
    auto result = CutImpl(pCutData, interactionEvent);
    if (result)
    {
        m_undoList.push(result);
        ClearRedo();
        Refresh();
    }   
}

void CutImplementation::Init(mitk::DataNode* pDataNode)
{
    if (!pDataNode)
    {
        return;
    }
    m_pDataNode = pDataNode;
    ClearUndo();
    ClearRedo();
    m_dataType = GetDataType(pDataNode);
    if (GetDataObject())
    {
        m_undoList.push(GetCopyOfDataObject());
    }
}

void CutImplementation::ClearUndo()
{
    while (!m_undoList.empty())
    {
        m_undoList.pop();
    }

}
void CutImplementation::ClearRedo()
{
    while (!m_redoList.empty())
    {
        m_redoList.pop();
    }

}

const vtkSmartPointer<vtkDataObject> CutImplementation::PopUndo()
{
    if (!m_undoList.empty())
    {
        auto data = m_undoList.top();
        m_undoList.pop();
        return data;
    }
    else
    {
        return nullptr;
    }
}

const vtkSmartPointer<vtkDataObject> CutImplementation::PopRedo()
{
    if (!m_redoList.empty())
    {
        auto data = m_redoList.top();
        m_redoList.pop();
        return data;
    }
    else
    {
        return nullptr;
    }
}

const vtkSmartPointer<vtkDataObject> CutImplementation::TopOfUndo()
{
    if (!m_undoList.empty())
    {
        auto data = m_undoList.top();
        return data;
    }
    else
    {
        return nullptr;
    }

}
const vtkSmartPointer<vtkDataObject> CutImplementation::TopOfRedo()
{
    if (!m_redoList.empty())
    {
        auto data = m_redoList.top();
        return data;
    }
    else
    {
        return nullptr;
    }
}

void CutImplementation::PushUndo(const vtkSmartPointer<vtkDataObject>& data)
{
    if (data)
    {
        m_undoList.push(data);
    }
}
void CutImplementation::PushRedo(const vtkSmartPointer<vtkDataObject>& data)
{
    if (data)
    {
        m_redoList.push(data);
    }
}

void CutImplementation::Undo()
{
    if (m_undoList.size() <= 1)
    {
        return;
    }
    auto data = PopUndo();
    if (data)
    {
        // Refresh(data);
        PushRedo(data);
        Refresh();
    }
}

void CutImplementation::Redo()
{
    auto data = PopRedo();
    if (data)
    {
        PushUndo(data);
        Refresh();
    }
}

void CutImplementation::Reset()
{
    vtkSmartPointer<vtkDataObject> data = nullptr;
    while (!m_undoList.empty())
    {
        data = m_undoList.top();
        m_undoList.pop();
        m_redoList.push(data);
    }
    Refresh();
}

void CutImplementation::Finished()
{
    ClearRedo();
    ClearUndo();
}


CutImplementation::DataType CutImplementation::GetDataType(mitk::DataNode* node)
{
    if (dynamic_cast<mitk::Surface *>(node->GetData()) != nullptr)
    {
        return Surface;
    }
    else if (dynamic_cast<mitk::Image *>(node->GetData()) != nullptr)
    {
        return Image;
    }
    else
    {
        return Unknown;
    }
}

vtkDataObject* CutImplementation::GetDataObject()
{
    if (m_dataType==Surface)
    {
        mitk::Surface* pData = dynamic_cast<mitk::Surface*>(m_pDataNode->GetData());
        if (pData)
        {
            return pData->GetVtkPolyData();
        }
        else
        {
            return nullptr;
        }
    }
    else if (m_dataType==Image)
    {
        mitk::Image* pData = dynamic_cast<mitk::Image*>(m_pDataNode->GetData());
        if (pData)
        {
            return pData->GetVtkImageData();
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        return nullptr;
    }

    
}
vtkSmartPointer<vtkDataObject> CutImplementation::GetCopyOfDataObject()
{
    if (m_dataType == Surface)
    {
        mitk::Surface* pData = dynamic_cast<mitk::Surface*>(m_pDataNode->GetData());
        if (pData)
        {
            auto copy = vtkSmartPointer<vtkPolyData>::New();
            copy->DeepCopy(pData->GetVtkPolyData());
            return copy;
        }
        else
        {
            return nullptr;
        }
    }
    else if (m_dataType == Image)
    {
        mitk::Image* pData = dynamic_cast<mitk::Image*>(m_pDataNode->GetData());
        if (pData)
        {
            auto copy = vtkSmartPointer<vtkImageData>::New();
            copy->DeepCopy(pData->GetVtkImageData());
            return copy;
        }
        else
        {
            return nullptr;
        }
    }
    else 
    {
        return nullptr;

    }
    

}
void CutImplementation::Refresh()
{
    if (m_dataType == Surface)
    {
        mitk::Surface* pData = dynamic_cast<mitk::Surface*>(m_pDataNode->GetData());
        auto poly = dynamic_cast<vtkPolyData*>(TopOfUndo().Get());
        if (pData&&poly)
        {
            pData->SetVtkPolyData(poly);
            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
    }
    else if (m_dataType == Image)
    {
        mitk::Image* pData = dynamic_cast<mitk::Image*>(m_pDataNode->GetData());
        auto img = dynamic_cast<vtkImageData*>(TopOfUndo().Get());
        if (pData&&img)
        {
            pData->SetVolume(img->GetScalarPointer());
            mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
    }
    
}