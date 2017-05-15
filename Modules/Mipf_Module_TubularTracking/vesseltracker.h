#ifndef __VesselTracker_H
#define __Vesseltracker_H

#define ML_DEBUG_ENV_NAME "VESSELTRACKER"
#include "util.h"

// For boost graph library
#include "VesselGraph.h"

// Newmat library for linear algebra
#include "newmat.h"
#include "newmatap.h"
#include <iostream>

// Standard C++ libraries
#include <vector>
#include <deque>

#define round(x) (x<0?ceil((x)-0.5):floor((x)+0.5))

// Tracking termination codes
#define NORMAL_TERMINATION          0 // Normal tracking termination (no more vessels to track)
#define BAD_INITIAL_MODEL           1 // Initial model does not fit image data.
#define MAX_NBR_STEPS_TAKEN         2 // Maximum number of tracking steps reached.
#define TOO_CLOSE_TO_BORDER         3 // Too close to image data border.
#define INSUFFICIENT_IMAGE_DATA     4 // Not enough image data supplied.
#define MAX_LENGTH_REACHED          5 // Maximum tracking length reached.
#define OUTSIDE_MASK                6 // Hit user-defined mask.



enum DIM {_2D=0, _3D=1};


class VesselTracker
{
public:

  // When the data pointer in maskData is NULL, no mask is supplied.
  VesselTracker(InternalImageType* imData, BinaryImageType* maskData,
                const MedicalImageProperties& imProps, DIM TRACKDIM);

  virtual ~VesselTracker();

  //! Perform tracking.
  unsigned int track(boostGraph& treeGraph,
                     Vertex initVertex,
                     std::pair<int,double>& trackingLength,               // <number of steps taken, length tracked in millimeters>
                     const std::pair<int,double>& maxTrackingLength,      // Max values for the above quantities
                     BinaryImageType& binaryImg);

  //! Get termination code message returned by VesselTracker::track()
  static const char* getTerminationMessage(unsigned int retCode);

  //! Set number of angles to sample.
  void setNbrSearchAngles(int nbrAngles) {_nbrAngles = nbrAngles;}

  //! Set maximum search angle in radians.
  void setMaxSearchAngle(double angle) {_maxSearchAngle = angle;}
  double getMaxSearchAngle(bool isBranchRegion) const {return (isBranchRegion)? _maxBranchRegionAngle : _maxSearchAngle;}

  //! Allow tracking to branch.
  void allowBranching() {_allowBranching = true;}

  //! No branching allowed, a single path will be tracked. In such case the speed of
  //! the algorithm can be increase by setting a lower number of search angles and
  //! a lower maximum search angle.
  void disallowBranching() {_allowBranching = false;}

  //! Set the minimum distance between to branchings in milltmeters.
  void setMinBranchingDistance(double minBranchingDistance) {_minBranchingDistance = minBranchingDistance;}

  //! Set depth of the search tree. Warning, a large depth (>4) may
  //! result in very long times.
  void setSearchDepth(int searchDepth) {_searchDepth = searchDepth;}

  //! Set the minimum score a vessel model must survive to be accepted valid.
  void setPruningThreshold(double pruningThreshold) {_pruningThreshold = pruningThreshold;}

  //! Set the average score a path of vessel models (of length _searchDepth)
  //! must survive to be accepted valid. This threshold should be higher than
  //! the threshold for a single step.
  void setTerminationThreshold(double terminationThreshold) {_terminationThreshold = terminationThreshold;}

  //! Set minimum vessel radius in millimeters. A lower limit on the radius
  //! is set by the voxel size; the radius cannot be smaller than 1/2 voxel.
  void setMinRadius(double minRadius);
  //! Get minimum radius.
  double getMinRadius(void) const {return _minRadius;}

  //! Set maximum vessel radius in millimeters.
  void setMaxRadius(double maxRadius) {_maxRadius = maxRadius;}
  //! Get maximum radius.
  double getMaxRadius(void) const {return _maxRadius;}

  //! Set step length in terms of a factor the current radius.
  void setStepLength(double factor) {_stepLengthFactor = factor;}
  //! Return the tracking step length.
  double getStepLength(double radius) const {return radius*_stepLengthFactor;}

  //! Returns true if the vessel model vd is not too close to the given image data border.
  bool borderCheck(const VesselData& vd);

  //! Returns true if the vessel model vd is within the user-given mask.
  bool withinMask(const VesselData& vd);

  //! Mark voxels that have been visited, i.e, voxelize a vessel model.
  virtual void setVisitedVoxels(const VesselData& vd, BinaryImageType& binaryImg) = 0;

protected:
  //void getVoxelSize(double& sx, double& sy, double& sz) const {sx = _worldVoxelSize[0]; sy = _worldVoxelSize[1]; sz = _worldVoxelSize[2];}

  //! Block of image data (or at least with a pointer to where the image data are).
  InternalImageType* _imData;

  //! Block of image mask data indicating where the tracking is not allowed to go.
  BinaryImageType* _maskData;

  //! Properties of the image data such as voxel size and methods for converting
  //! between the world coordinate system and the voxel coordinate system.
  const MedicalImageProperties& _imProps;

  //! The voxel size in millimeters. This can be obtained from _imProps, but since the voxel
  //! size is extensively used we store it a separate variable.
  double _worldVoxelSize[3];
  double _largestVoxelSize;
  double _smallestVoxelSize;

  //! _TRACKDIM is either _2D or _3D to indicate whether tracking is performed in 2D or 3D.
  DIM _TRACKDIM;

private:

  //! Levenberg-Marquardt optimization of the parameters.
  virtual void _fitModel(VesselData& vd, const VesselData& vd_prev) = 0;

  //! Calculate score of a vessel template, i.e., how well it fits the image
  virtual void _calcScore(VesselData& vd)  = 0;

  //! Determines a suitable template size in voxels. The half size is returned for
  //! each dimension (x,y,z) is returned. That is, if n is the half side, 2*n+1 is
  //! the full size.
  virtual void _getPatchSize(int voxelHalfSize[3], double radius) = 0;

  //! Produce guesses of new vessel templates from the vessel template vt.
  void _generateGuesses(std::vector<VesselData>& guesses, const VesselData& vd);

  //! Analyze the scores of a set of vessel template guesses.
  void _analyzeScores(std::vector<VesselData>& bestGuesses, const std::vector<VesselData>& guesses);

  //! Extends the tracking tree with one level.
  void _extendGraph(const Vertex& rootVertex, VertexScorePairs& trialLeafs);

  //! Detects if a branching has been passed by performing a spectral clustering.
  bool _detectBranching(std::vector<Vertex>& branchLeafs, const VertexScorePairs& leafs);

  //! Calculates the distance between the center points of the tubular templates in graph vertexes v1 and v2.
  double _calcWorldDistance(const Vertex& v1, const Vertex& v2, const boostGraph& graph) ;

  //! Add a vertex to the graph treeGraph.
  Vertex _addTreeGraphVertex(boostGraph& treeGraph,
                             std::pair<int,double>& trackingLength,
                             BinaryImageType& binaryImg,
                             const Vertex& currentTreeVertex,
                             const VesselData& vd);

  //! Branch graph. A boost graph object that where all the steps tried out
  // during tracking are stored.
  boostGraph _branchGraph;

  //! Set parameters to track under normal or branching assumptions. This means that a branching has
  //! been detected and the max search angle is made smaller until the branching has been
  //! passed so that the same junction is not detected twice.
  void _setTrackingState(bool isBranchState);

  //! Are we tracking in a branching state or a normal state?
  bool _branchingState;

  //! Threshold for a single step.
  double _pruningThreshold;

  //! Threshold for a path of length _searchDepth.
  double _terminationThreshold;

  //! Depth of search tree.
  int _searchDepth;

  //! Indicates whether the algorithm should search for branches.
  bool _allowBranching;

  //! Number of angles to check on each side of a vessel template direction
  //! when producing guesses of the next center point. That is, the total
  //! number of guesses are 2*_nbrAngles+1 in 2D and (2*_nbrAngles+1)^2 in 3D.
  int _nbrAngles;

  //! Max search angle in radians.
  double _maxSearchAngle;

  //! Max search angle when we just passed a branching (in radians).
  double _maxBranchRegionAngle;

  //! Max radius,
  double _maxRadius;

  //! Min radius.
  double _minRadius;

  //! Factor that determines the step length. The step length is calculated
  //! as the current radius times this factor.
  double _stepLengthFactor;

  //! Set the minimum distance between to branchings in millimeters.
  double _minBranchingDistance;
};

#endif
