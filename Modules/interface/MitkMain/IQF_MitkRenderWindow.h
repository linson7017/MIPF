#ifndef IQF_MitkRenderWindow_h__
#define IQF_MitkRenderWindow_h__

#include <QString>
#include <QSet>
class QmitkRenderWindow;
class QmitkStdMultiWidget;

namespace mitk
{
    class RenderingManager;
    class DataNode;
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
    virtual void SetMitkStdMultiWidget(QmitkStdMultiWidget* stdMultiWidget,const QString& id="") = 0;
   
    //functions
    virtual QmitkStdMultiWidget* GetMitkStdMultiWidget(const QString& id = "") = 0;
    virtual QmitkRenderWindow* GetActiveMitkRenderWindow() = 0;
    virtual mitk::RenderingManager* GetRenderingManager(QString name = "") = 0;

    virtual void SetCrossHairVisibility(bool state,const QString& id="") = 0;
    virtual void ResetCrossHair(const QString& id = "") = 0;

    virtual void Reinit(mitk::DataNode* node) = 0;
};
#endif // IQF_MitkRenderWindow_h__