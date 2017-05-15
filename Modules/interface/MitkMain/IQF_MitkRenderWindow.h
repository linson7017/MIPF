#ifndef IQF_MitkRenderWindow_h__
#define IQF_MitkRenderWindow_h__

#include <QString>
#include <QSet>
class QmitkRenderWindow;
class QmitkStdMultiWidget;

namespace mitk
{
    class RenderingManager;
}

const char QF_MitkMain_RenderWindow[] = "QF_MitkMain_RenderWindow";

class IQF_MitkRenderWindow
{
public:
    //manage windows as map
    virtual QHash<QString, QmitkRenderWindow*> GetQmitkRenderWindows()=0;
    virtual QmitkRenderWindow* GetQmitkRenderWindow(const QString& id)=0;

    //handle windows
    virtual void AddMitkRenderWindow(QmitkRenderWindow* renderWindow, const QString& id) = 0;
    virtual void RemoveMitkRenderWindow(const QString& id) = 0;
    virtual void SetMitkStdMultiWidget(QmitkStdMultiWidget* stdMultiWidget) = 0;
   
    //functions
    virtual QmitkStdMultiWidget* GetMitkStdMultiWidget() = 0; 
    virtual QmitkRenderWindow* GetActiveMitkRenderWindow() = 0;
    virtual mitk::RenderingManager* GetRenderingManager(QString name = "") = 0;

    virtual void SetCrossHairVisibility(bool state) = 0;
};
#endif // IQF_MitkRenderWindow_h__