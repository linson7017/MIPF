#ifndef SurfaceExtractView_h__
#define SurfaceExtractView_h__

#pragma once

#include "MitkPluginView.h"
#include <QObject>
#include "ITKImageTypeDef.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentRun>

class QmitkDataStorageComboBox;


class SurfaceExtractView :public QObject, public MitkPluginView
{
    Q_OBJECT
public:
    SurfaceExtractView(QF::IQF_Main* pMain);
    ~SurfaceExtractView();
    void Constructed(R* pR);
protected:
    virtual void Update(const char* szMessage, int iValue = 0, void* pValue = 0);
    void OnSurfaceCalculationDone();
protected slots:
    void ExtractSmoothedSurface(mitk::Image* image);
    void ExtractSurface(mitk::Image* image,int smooth=0, bool largestConnect = false);
    void ShowResult();


    void GetForeground( Float3DImageType* image, UChar3DImageType* outputImage);

private:
    QmitkDataStorageComboBox* m_pImageSelector;
    QmitkDataStorageComboBox* m_pSurfaceSelector;
    mitk::DataStorage::Pointer m_pDataStorage;
    mitk::Surface::Pointer m_pSurface;

    QFuture<void> m_Future;
    QFutureWatcher<void> m_Watcher;

    QString m_SurfaceName;

    float m_smoothing;
    bool m_smoothingHint ;
    float m_decimation;
    float m_closing;
    int m_timeNr;
};

#endif // SurfaceExtractView_h__
