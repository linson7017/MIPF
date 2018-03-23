#include "CQF_MitkRenderWindow.h"
#include <QmitkStdMultiWidget.h>
#include <mitkVtkPropRenderer.h>
#include <mitkRenderingManager.h>
#include <QmitkRenderWindow.h>

CQF_MitkRenderWindow::CQF_MitkRenderWindow():m_DefaultMultiWidgetID("")
{

}

CQF_MitkRenderWindow::~CQF_MitkRenderWindow()
{

}

QSet<QmitkRenderWindow*> CQF_MitkRenderWindow::GetMitkRenderWindowSet()
{
    return m_RenderWindows;
}

void CQF_MitkRenderWindow::SetMitkRenderWindowSet(QSet<QmitkRenderWindow*> renderWindows)
{
    m_RenderWindows = renderWindows;
}

QmitkStdMultiWidget* CQF_MitkRenderWindow::GetMitkStdMultiWidget(const QString& id)
{
    QString tempID = id;
    if (tempID.isEmpty())
    {
        tempID = m_DefaultMultiWidgetID;
    }
    return m_StdMultiWidgetMap.value(tempID, NULL);
}

void CQF_MitkRenderWindow::SetMitkStdMultiWidget(QmitkStdMultiWidget* stdMultiWidget, const QString& id)
{  
    if (!stdMultiWidget)
    {
        return;
    }
    if (id.isEmpty())
    {
        m_StdMultiWidgetMap.insert(m_DefaultMultiWidgetID,stdMultiWidget);
    }
    else
    {
        m_StdMultiWidgetMap.insert(id, stdMultiWidget);
    }
    
    for (int i=0;i<4;i++)
    {
        m_RenderWindows.insert(stdMultiWidget->GetRenderWindow(i));
    }
    m_RenderWindowMap.insert(id+"-axial", stdMultiWidget->GetRenderWindow1());
    m_RenderWindowMap.insert(id + "-sagittal", stdMultiWidget->GetRenderWindow2());
    m_RenderWindowMap.insert(id + "-coronal", stdMultiWidget->GetRenderWindow3());
    m_RenderWindowMap.insert(id + "-3d", stdMultiWidget->GetRenderWindow4());
}

mitk::RenderingManager* CQF_MitkRenderWindow::GetRenderingManager(QString name)
{
    return mitk::RenderingManager::GetInstance();
}

QmitkRenderWindow* CQF_MitkRenderWindow::GetActiveMitkRenderWindow()
{
    QmitkRenderWindow* renderWindow = NULL;
    QHash<QString,QmitkRenderWindow *>::const_iterator itor = m_RenderWindowMap.constBegin();
    while (itor != m_RenderWindowMap.constEnd()) {
        if ((*itor)->isActiveWindow())
        {
            renderWindow = *itor;
            break;
        }
        ++itor;
    }
    return renderWindow;
}

QHash<QString, QmitkRenderWindow*> CQF_MitkRenderWindow::GetQmitkRenderWindows()
{
    return m_RenderWindowMap;
}

QmitkRenderWindow* CQF_MitkRenderWindow::GetQmitkRenderWindow(const QString& id)
{
    if (m_RenderWindowMap.contains(id))
        return m_RenderWindowMap[id];

    return 0;
}

void CQF_MitkRenderWindow::AddMitkRenderWindow(QmitkRenderWindow* renderWindow, const QString& id)
{
    m_RenderWindowMap.insert(id, renderWindow);
}

void CQF_MitkRenderWindow::RemoveMitkRenderWindow(const QString& id)
{
    m_RenderWindowMap.remove(id);
}

void CQF_MitkRenderWindow::SetCrossHairVisibility(bool state, const QString& id)
{
    if (id.isEmpty())
    {
        foreach(QmitkStdMultiWidget* widget,m_StdMultiWidgetMap.values())
        {
            widget->GetWidgetPlane1()->SetVisibility(state);
            widget->GetWidgetPlane2()->SetVisibility(state);
            widget->GetWidgetPlane3()->SetVisibility(state);
        }
    }
    else
    {
        QmitkStdMultiWidget* widget = m_StdMultiWidgetMap.value(id, NULL);
        if (widget)
        {
            widget->GetWidgetPlane1()->SetVisibility(state);
            widget->GetWidgetPlane2()->SetVisibility(state);
            widget->GetWidgetPlane3()->SetVisibility(state);
        }
    }
    GetRenderingManager()->RequestUpdateAll();
}

void CQF_MitkRenderWindow::ResetCrossHair(const QString& id)
{
    if (id.isEmpty())
    {
        foreach(QmitkStdMultiWidget* widget, m_StdMultiWidgetMap.values())
        {
            widget->ResetCrosshair();
        }
    }
    else
    {
        QmitkStdMultiWidget* widget = m_StdMultiWidgetMap.value(id, NULL);
        if (widget)
        {
            widget->ResetCrosshair();
        }
    }
}

void CQF_MitkRenderWindow::Reinit(mitk::DataNode* node)
{
    if (!node)
    {
        return;
    }
    mitk::BaseData::Pointer basedata = node->GetData();
    if (basedata.IsNotNull() &&
        basedata->GetTimeGeometry()->IsValid())
    {
        GetRenderingManager()->InitializeViews(
            basedata->GetTimeGeometry(), mitk::RenderingManager::REQUEST_UPDATE_ALL, true);
    }
}