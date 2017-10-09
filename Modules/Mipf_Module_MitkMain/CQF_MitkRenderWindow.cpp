#include "CQF_MitkRenderWindow.h"
#include <QmitkStdMultiWidget.h>
#include <mitkVtkPropRenderer.h>
#include <mitkRenderingManager.h>
#include <QmitkRenderWindow.h>

CQF_MitkRenderWindow::CQF_MitkRenderWindow()
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

QmitkStdMultiWidget* CQF_MitkRenderWindow::GetMitkStdMultiWidget()
{
    return m_StdMultiWidget;
}

void CQF_MitkRenderWindow::SetMitkStdMultiWidget(QmitkStdMultiWidget* stdMultiWidget)
{
    m_StdMultiWidget = stdMultiWidget;
    for (int i=0;i<4;i++)
    {
        m_RenderWindows.insert(m_StdMultiWidget->GetRenderWindow(i));
    }

    m_RenderWindowMap.insert("axial", m_StdMultiWidget->GetRenderWindow1());
    m_RenderWindowMap.insert("sagittal", m_StdMultiWidget->GetRenderWindow2());
    m_RenderWindowMap.insert("coronal", m_StdMultiWidget->GetRenderWindow3());
    m_RenderWindowMap.insert("3d", m_StdMultiWidget->GetRenderWindow4());
}

mitk::RenderingManager* CQF_MitkRenderWindow::GetRenderingManager(QString name)
{
    return mitk::RenderingManager::GetInstance();
    /*QmitkRenderWindow* renderWindow = GetActiveMitkRenderWindow();

    if (renderWindow)
    {
        return renderWindow->GetRenderer()->GetRenderingManager();
    }
    else
    {
        return NULL;
    }*/
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

void CQF_MitkRenderWindow::SetCrossHairVisibility(bool state)
{
    if (m_StdMultiWidget)
    {
        mitk::DataNode *n;
        if (this->m_StdMultiWidget)
        {
            n = this->m_StdMultiWidget->GetWidgetPlane1();
            if (n)
                n->SetVisibility(state);
            n = this->m_StdMultiWidget->GetWidgetPlane2();
            if (n)
                n->SetVisibility(state);
            n = this->m_StdMultiWidget->GetWidgetPlane3();
            if (n)
                n->SetVisibility(state);
            GetRenderingManager()->RequestUpdateAll();
        }
    }
}

void CQF_MitkRenderWindow::ResetCrossHair()
{
    if (m_StdMultiWidget)
    {
        m_StdMultiWidget->ResetCrosshair();
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