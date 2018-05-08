#ifndef ImageAlgorithm_H
#define ImageAlgorithm_H

#include <list>
#include <vector>
#include "vtkResliceImageViewerMeasurements.h"
#include "vtkDICOMImageReader.h"
#include "vtkActor.h"
#include "vtkImageThreshold.h"
#include "vtkImageThresholdConnectivity.h"
#include "vtkMarchingCubes.h"
#include "vtkDiscreteMarchingCubes.h"
#include "vtkWindowedSincPolyDataFilter.h"
#include "vtkPolyDataMapper.h"
#include "vtkSphereSource.h"
#include "vtkParametricSpline.h"
#include "vtkMutableDirectedGraph.h"
#include "vtkGraphToPolyData.h"
#include "vtkDijkstraGraphGeodesicPath.h"
#include "vtkContourWidget.h"
#include "vtkProperty.h"
#include "vtkOrientedGlyphContourRepresentation.h"
#include "vtkImageDilateErode3D.h"
#include "vtkImageOpenClose3D.h"
#include "vtkImageResample.h"
#include "vtkImageStencil.h"
#include "vtkIntArray.h"

struct AneurysmData
{
    vtkSmartPointer<vtkPolyData> aneurysmSurfaceData;
    vtkSmartPointer<vtkPolyData> aneurysmDisplayPolyData;
    vtkSmartPointer<vtkPoints> seeds;
    double diameter;
    double height;
    double neckDiameter;
    double volume;
    double inflowAngle;
	double neckcenterpoint[3];
	double farpoint[3];
	double pathcenterpoint[3];
	double heightpoint[3];
	double aneurysmPoint1[3];
	double normalDirPoint[3];
	double vesselRadius;
	double microtubulePoint[3];
};

struct PatientInfo
{
	std::string seriesInstanceUID;
	std::string patientID;
	std::string modality;
	std::string studyID;
	std::string seriesNumber;
	std::string contentDate;
	std::string contentTime;
	std::string patientName;
	std::string institute;
	std::string medicalRecordLocator;
};

class ImageAlgorithm
{

public:
	ImageAlgorithm();
	PatientInfo patientInfo;
	void destroy();
	bool init(vtkSmartPointer<vtkImageData> img);
	vtkSmartPointer<vtkImageData> getInputData();
    void SetInputData(vtkSmartPointer<vtkImageData> img) { inputData = img; }
	vtkSmartPointer<vtkImageData> getResampledInputData();
	
	int getImageType();
	double getLimitLow();
	double getLimitHigh();
	int getImageDims(int dim);

	void autoSelectRange();

	vtkSmartPointer<vtkPolyData> getCurrentSurface();
	vtkSmartPointer<vtkPolyData> getCurrentSurfaceShowing();
	void setCurrentSurfaceShowing(vtkSmartPointer<vtkPolyData> surfaceDataShowing);

	void setLowerth(double minValue);
	void setUpperth(double maxValue);
	double getLowerth();
	double getUpperth();

	double getThreshMinimum();
	double getThreshMaximum();
  
	int addSeed(double(&x)[3]);
	int removeSeed(double(&x)[3]);
	void resetSeed();
	int setStartEndPoint(int order, double(&x)[3]);
	bool isPathReady();
	bool isStartPointTooFar;
	bool isEndPointTooFar;
	bool isStartEndPointTooClose;
	bool isAneurysmPointTooFar;
	bool isAneurysmLocPointTooFar;
    
    bool addAneurysmPoint(double x[3]);
	bool setAneurysmLocationPoint(double x[3]);
    
    AneurysmData detectAneurysm();

	void setVesselnessThreshold(double threshold);

    vtkSmartPointer<vtkPolyData> generateSurface();
	vtkSmartPointer<vtkPolyData> pruneVesselTree();
    vtkSmartPointer<vtkPolyData> generateExtensions();
	vtkSmartPointer<vtkPolyData> generateRectSurface();
	vtkSmartPointer<vtkPolyData> generateVesselWallSurface();
	vtkSmartPointer<vtkPolyData> generateNeedle();
	vtkSmartPointer<vtkPolyData> generateMould(vtkSmartPointer<vtkOrientedGlyphContourRepresentation> needleContourRepresentation);

	vtkSmartPointer<vtkPoints> getCenterPoints();

	void setWallThickness(int thickness);
	void setPruneRadius(int radius);

    vtkSmartPointer<vtkImageData> getSegmentedData() { return segmentedData; }

private: 
	int imageType; // 0 - DSA, 1 - CTA, 2 - MRA
	double lowerth;
	double upperth;
	double defaultLowerth;
	double threshMinimum;
	double threshMaximum;
	int imageDims[3];
	double valuesRange[2];
	int pruneRadius;
	int wallThickness;
	double vesselnessThreshold;
	int firstAneurysmLocPointIdx = -1;
	int lastAneurysmLocPointIdx = -1;
    vtkSmartPointer<vtkIntArray> skeletonNeighbors;
    vtkSmartPointer<vtkPolyData> currentSurface;
	vtkSmartPointer<vtkPolyData> currentSurfaceShowing;
    
    AneurysmData aneurysmData;

    vtkSmartPointer<vtkImageData> pruneVesselData;
    vtkSmartPointer<vtkImageData> vesselExtensionsData;
	vtkSmartPointer<vtkImageData> vesselness;
	vtkSmartPointer<vtkImageData> enhancedVesselData;
    vtkSmartPointer<vtkImageData> aneurysmImageData;
	vtkSmartPointer<vtkPoints> seedPoints;
	vtkSmartPointer<vtkPoints> startEndPoints;
    vtkSmartPointer<vtkPoints> aneurysmPoints;
	vtkSmartPointer<vtkPoints> aneurysmLocationPoints;
    vtkSmartPointer<vtkImageData> segmentedData;
	vtkSmartPointer<vtkImageData> segmentedDataBackup;
	vtkSmartPointer<vtkImageData> inputData;
	vtkSmartPointer<vtkPolyData> needleData;
	vtkSmartPointer<vtkPolyData> moulddata;
    vtkSmartPointer<vtkImageData> tubeImageData;
	
	//vtkSmartPointer<vtkMarchingCubes> mouldsurface;
	vtkSmartPointer<vtkDiscreteMarchingCubes> discreteSurface;
	vtkSmartPointer<vtkWindowedSincPolyDataFilter> smoother;
	
	vtkSmartPointer<vtkMutableDirectedGraph> skeletonGraph;
	vtkSmartPointer<vtkGraphToPolyData> graphToPolyData;
	vtkSmartPointer<vtkDijkstraGraphGeodesicPath> dijkstra;
	
	vtkSmartPointer<vtkImageResample> resampler;

    void segmentVessels();
    void generateSkeleton();
	void generateShortestPath();
	void generateShortestPathforSinglePoint(int order);
	void getNeedleDirection(int, vtkSmartPointer<vtkImageData>, int[3], int[3], vtkSmartPointer<vtkPolyData>, double[3]);
    void calculateVesselness();
	void vesselEnhance();
    void generateEndSupport();
    void generateTube(double[3], double[3], double, double);

    bool needRecalculateSegmentation;
    bool needRecalculateSkeleton;
	bool needRecalculateShortestPath;
    bool needRecalculateEndSupport;
	bool needRecalculateVesselness;
    bool needRecalculateEnhancedData;
    
    bool displayCenterLine;
};

#endif // ImageAlgorithm_H
