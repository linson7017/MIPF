/********************************************************************
	FileName:    LSSegmentation.h
	Author:        Ling Song
	Date:           Month 10 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef LSSegmentation_h__
#define LSSegmentation_h__

#include <QObject>
#include <mitkImage.h>
#include <mitkDataStorage.h>
#include <mitkPointSet.h>
#include <mitkSurface.h>
#include <vtkPolyData.h>
#include "ITKImageTypeDef.h"

//itk
#include "itkGeodesicActiveContourLevelSetImageFilter.h"

namespace QF
{
    class IQF_Main;
}


class LSSegmentation : public QObject
{
    Q_OBJECT
public:
    LSSegmentation(mitk::DataStorage* pDataStorage, QF::IQF_Main* pMain);
    ~LSSegmentation();
    template<class TFilter>
    class CommandIterationUpdate;

    struct Parameters
    {
        Parameters()
        {
            Alpha = 40.0;
            Beta = 40.0;
            PropagationScaling = 1.0;
            AdvectionScaling = 1.0;
            CurvatureScaling = 1.0;
            StoppingValue = 150;
            NumberOfInteraction = 500;
        }
        double Alpha;
        double Beta;
        double PropagationScaling;
        double AdvectionScaling;
        double CurvatureScaling;
        double StoppingValue;
        int NumberOfInteraction;
    };

    Parameters PARAMETERS;
    public slots:
    void SlotDoSegmentation(const mitk::Image::Pointer& inputImage, const  mitk::PointSet::Pointer& pSeedPointSet);
    void SlotStopSegmentation();
signals:
    void SignalInteractionEnd(const mitk::Image::Pointer& image);
    void SignalSegmentationFinished();
private:
    mitk::DataStorage* m_pDataStorage;
    QF::IQF_Main* m_pMain;

    typedef itk::GeodesicActiveContourLevelSetImageFilter<Float3DImageType, Float3DImageType> LevelSetType;
    LevelSetType::Pointer m_levelFilter;
};

#endif // LSSegmentation_h__