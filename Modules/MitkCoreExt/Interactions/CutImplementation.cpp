#include "CutImplementation.h"

#include <vtkMetaImageWriter.h>

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