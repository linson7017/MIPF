/********************************************************************
	FileName:    LSASMSegmentation.h
	Author:        Ling Song
	Date:           Month 10 ; Year 2017
	Purpose:	     
*********************************************************************/
#ifndef LSASMSegmentation_h__
#define LSASMSegmentation_h__


#include <QObject>
#include <QVector>
#include <mitkImage.h>
#include <mitkDataStorage.h>
#include <mitkPointSet.h>
#include <mitkSurface.h>
#include "vtkPolyData.h"

namespace QF
{
    class IQF_Main;
}

class LSASMSegmentation : public QObject
{
    Q_OBJECT
public:
    LSASMSegmentation(mitk::DataStorage* pDataStorage,QF::IQF_Main* pMain);
    ~LSASMSegmentation();

    template<class TFilter>
    class CommandIterationUpdate;

    public slots:
    void SlotDoSegmentation(mitk::Image* inputImage,mitk::Image* inputMeanImage,mitk::PointSet* pSeedPointSet, const QVector<mitk::Image*>& pcaImageList,mitk::Image* outputImage);

signals:
    void SignalInteractionEnd(vtkPolyData* tempImage);
private:
    mitk::DataStorage* m_pDataStorage;
    QF::IQF_Main* m_pMain;
};

#endif // LSASMSegmentation_h__