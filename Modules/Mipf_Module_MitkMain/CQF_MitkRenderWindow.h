#ifndef CQF_MitkRenderWindow_h__
#define CQF_MitkRenderWindow_h__

#pragma once

#include "MitkMain/IQF_MitkRenderWindow.h"
#include <mitkStandaloneDataStorage.h>



class CQF_MitkRenderWindow : public IQF_MitkRenderWindow
{
public:
    CQF_MitkRenderWindow();
    ~CQF_MitkRenderWindow();
    virtual  QSet<QmitkRenderWindow*> GetMitkRenderWindowSet();
    virtual void SetMitkRenderWindowSet(QSet<QmitkRenderWindow*> renderWindows);

    virtual QHash<QString, QmitkRenderWindow*> GetQmitkRenderWindows();
    virtual QmitkRenderWindow* GetQmitkRenderWindow(const QString& id);

    virtual void AddMitkRenderWindow(QmitkRenderWindow* renderWindow, const QString& id);
    virtual void RemoveMitkRenderWindow(const QString& id);

    virtual QmitkStdMultiWidget* GetMitkStdMultiWidget();
    virtual void SetMitkStdMultiWidget(QmitkStdMultiWidget* stdMultiWidget);
    virtual mitk::RenderingManager* GetRenderingManager(QString name="");
    virtual QmitkRenderWindow* GetActiveMitkRenderWindow();

    virtual void SetCrossHairVisibility(bool state);

    virtual void ResetCrossHair();

    virtual void Reinit(mitk::DataNode* node);
private:
    QSet<QmitkRenderWindow*> m_RenderWindows;
    QHash<QString, QmitkRenderWindow*> m_RenderWindowMap;
    QmitkStdMultiWidget* m_StdMultiWidget;
};
#endif // CQF_MitkRenderWindow_h__