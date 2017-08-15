#ifndef SurfaceExtractView_h__
#define SurfaceExtractView_h__

#pragma once

#include "MitkPluginView.h"
#include <QWidget>
#include "ITKImageTypeDef.h"

#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentRun>

#include "ui_SurfaceExtractView.h"

class QmitkDataStorageComboBox;


class SurfaceExtractView :public QWidget, public MitkPluginView
{
    Q_OBJECT
public:
    SurfaceExtractView();
    ~SurfaceExtractView();
    void CreateView();
protected:
    void OnSurfaceCalculationDone();
protected slots:
    void Extract();
    void Smooth();
    void Simplify();
    void ExtractSurface(mitk::Image* image,int smooth=0, bool largestConnect = false);
    void ShowResult();
    void GetForeground( Float3DImageType* image, UChar3DImageType* outputImage);

private:
    mitk::Surface::Pointer m_pSurface;

    QFuture<void> m_Future;
    QFutureWatcher<void> m_Watcher;

    QString m_SurfaceName;

    float m_smoothing;
    bool m_smoothingHint ;
    float m_decimation;
    float m_closing;
    int m_timeNr;

    Ui::SurfaceExtractView m_ui;
};

#endif // SurfaceExtractView_h__
