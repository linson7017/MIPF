#ifndef VesselnessFilteringView_h__
#define VesselnessFilteringView_h__

#include "MitkPluginView.h"
#include <QWidget>

#include "ui_VesselFiltering.h"

//mitk
#include "mitkPointSet.h"

#pragma once
class VesselnessFilteringView:public QWidget,public MitkPluginView
{
public:
    VesselnessFilteringView();
    ~VesselnessFilteringView();
    void CreateView();

protected:
    void Start();

    double  alphaFromSuppressPlatesPercentage(double suppressPlatesPercentage);
    double  betaFromSuppressBlobsPercentage(double suppressBlobsPercentage);
    void computeVesselnessVolume(mitk::Image* currentVolumeNode, mitk::Image*  currentOutputVolumeNode,
        mitk::Point3D previewRegionCenterRAS, int previewRegionSizeVoxel = -1, int minimumDiameterMm = 0, int maximumDiameterMm = 25,
        double alpha = 0.3, double beta = 0.3, int contrastMeasure = 150);
    mitk::Point3D ConvertFromWorldToIndex(mitk::Image* volume, const mitk::Point3D& worldPoint);
private:
    Ui::VesselFilteringView m_ui;
    mitk::PointSet::Pointer m_seeds;
};


#endif // VesselnessFilteringView_h__
