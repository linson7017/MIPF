#ifndef TUBULARTRACKING_H
#define TUBULARTRACKING_H

#include "util.h"
#include "VesselTemplateTracker.h"
#include "VesselTemplate.h"
#include "VesselGraph.h"
#include "XMarkerGraph.h"
// Newmat includes
#include <newmatap.h>

#include "IQF_TubularTracking.h"

class TubularTracking : public IQF_TubularTracking
{
public:
    TubularTracking();
    ~TubularTracking();
    void SetInputImage(ImageType image);
    void SetParameterField(ParameterField& parameterField);
    void Track() { handleNotification(); }
    
    void SetSeedPoints(XMarkerList& list) { _initPointsXMarkerList = list; }
    std::vector<std::pair<XMarker, boostGraph> > getTrackedGraph() { return _trackedBoostGraphs; }
    void GetOutput(std::vector< std::vector<Vector3> >& result);
private:
    virtual void handleNotification();
  //! Minimum vessel radius.
  double _minRadiusFld;

  //! Maximum vessel radius.
  double _maxRadiusFld;

  //! Initial vessel radius.
  double _initRadiusFld;

  //! Use initial vessel radius.
  bool _useInitRadiusFld;

  //! Maximum angle relative to the previous tracking step (in degrees).
  double _maxAngleFld;

  //! Maximum number of search angles.
  int _nbrSearchAnglesFld;

  //! Score threshold that determines when tracking should be terminated.
  double _pruningThresholdFld;

  //! Score threshold that determines when tracking should be terminated.
  double _terminationThresholdFld;

  //! Maximum number of tracking steps.
  int _maxNbrStepsFld;

  //! Maximum tracking length in millimeters.
  double _maxLengthFld;

  //! How deep should the search tree be?.
  int _searchDepthFld;

  //! Minimum distance between two branchings in millimeters.
  double _minBranchingDistanceFld;

  //! Tracking step length. The step length is calculated
  //! by multiplying this factor with the current vessel radius.
  double _stepLengthFactorFld;

  //! Size factor for the (Gaussian) weight window used for fitting
  //! the vessel model.
  double _windowSizeFactorFld;

  //! Single hypothesis or multiple hypothesis tracking.
  bool  _useMultipleHypothesesTrackingFld;

  //! Toggle field for (dis-)allowing branchings in the tracking process.
  bool  _allowBranchingFld;

  //! Toggle field for growing bidirectional instead of in just the direction
  //! given by the user.
  bool _growBidirectionalFld;

  //! Toggle field for enabling/disabling tracking termination after a certain number of steps.
  bool _toggleMaxStepsFld;

  //! Toggle field for enabling/disabling tracking termination after a certain length.
  bool _toggleMaxLengthFld;

  bool _needInvertIntensityFld;

  //! List containing initial points.
  XMarkerList _initPointsXMarkerList;

  //! List containing the tracked points.
  XMarkerList _trackedPointsXMarkerList;

  //! Segmented binary image.
  BinaryImageType* _binaryImg;

  InternalImageType* _inputImage;

  //! Variable that indicates if output data has changed. Used to suppress unnecessary touch().
  bool _outputChanged;

  //! Get block of image data
  void _getImageData(InternalImageType &imData, BinaryImageType &maskData, const MedicalImageProperties &imgProps, double worldCenterPoint[3], int blockSize,  DIM trackDim);

  //! A vector of pairs containing an XMarker and a boost graph with a vessel
  //! tree tracked starting from the XMarker position.
  std::vector<std::pair<XMarker, boostGraph> > _trackedBoostGraphs;
  std::vector<std::pair<XMarker, XMarkerGraph>> _outputGraphs;

  Vector3 estimateOrientation(const Vector3& pos, const MedicalImageProperties& imgProps, InternalImageType& virtDataVol,
                              double radius, DIM trackDim);\
  void clearBinaryImage(BinaryImageType* image);

  void clearImage(InternalImageType* image);

  void copySubImage(InternalImageType* image, InternalImageType* srcImage);

  void invertIntensity(InternalImageType* image);



};

#endif // TUBULARTRACKING_H
