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
    Q_OBJECT
public:
    VesselnessFilteringView();
    ~VesselnessFilteringView();
    void CreateView();

protected slots:
    void Start();

protected:


    double  alphaFromSuppressPlatesPercentage(double suppressPlatesPercentage);
    double  betaFromSuppressBlobsPercentage(double suppressBlobsPercentage);
    void computeVesselnessVolume(mitk::Image* currentVolumeNode, mitk::Image*  currentOutputVolumeNode,
        mitk::Point3D previewRegionCenterRAS, int previewRegionSizeVoxel = -1, double minimumDiameterMm = 0, double maximumDiameterMm = 25,
        double alpha = 0.3, double beta = 0.3, double contrastMeasure = 150);
    mitk::Point3D ConvertFromWorldToIndex(mitk::Image* volume, const mitk::Point3D& worldPoint);
    double calculateContrastMeasure(vtkImageData* image, mitk::Point3D ijk, double diameter);
    double  getDiameter(vtkImageData* image, mitk::Point3D ijk);
    void calculateParameters();
private:
    Ui::VesselFilteringView m_ui;
    mitk::PointSet::Pointer m_seeds;

    double m_contrast;
};


#endif // VesselnessFilteringView_h__
