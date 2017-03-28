#ifndef MultiViews_H
#define MultiViews_H
//#include <qmainwindow.h>
#include <QWidget>
#include <mitkImage.h>
#include <mitkPointSet.h>
#include <mitkStandaloneDataStorage.h>
#include <itkImage.h>
#ifndef DOXYGEN_IGNORE

class QmitkStdMultiWidget;

class MultiViews : public QWidget
{
    Q_OBJECT
public:
    MultiViews(QWidget *parent = nullptr);
    ~MultiViews() {}
    virtual void Initialize();

    void ChangeLayout(int index);
    void ResetView();

    void SetDataStorage(mitk::DataStorage::Pointer dataStorage);
protected:
    virtual void SetupWidgets();
    mitk::DataStorage::Pointer m_DataStorage;
    mitk::Image::Pointer m_currentImage;
    mitk::PointSet::Pointer m_Seeds;
    mitk::Image::Pointer m_ResultImage;
    mitk::DataNode::Pointer m_ResultNode;

    QmitkStdMultiWidget* m_multiWidget;

    bool m_bInited;
};
#endif // DOXYGEN_IGNORE
#endif // MultiViews_H