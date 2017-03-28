#ifndef __VesselTemplateTracker_H
#define __VesselTemplateTracker_H

#include "VesselTracker.h"
#include "ImPatch.h"
#include "VesselTemplate.h"

class VesselTemplateTracker : public VesselTracker
{
public:
  // When the data pointer in maskData is NULL, no mask is supplied.
    VesselTemplateTracker(InternalImageType* imData, BinaryImageType* maskData,
                          const MedicalImageProperties& imProps, DIM TRACKDIM);

  virtual ~VesselTemplateTracker() {};

  //! Mark voxels that have been visited, i.e, voxelize a vessel model.
  virtual void setVisitedVoxels(const VesselData& vd, BinaryImageType& binaryImg);

  //! Sets the width factor of the Gaussian weight window. sigma = vesselRadius*windowSizeFactor.
  void setWindowSizeFactor(double windowSizeFactor) {_windowSizeFactor = windowSizeFactor;}

private:

  //! Levenberg-Marquardt optimization of the parameters.
  virtual void _fitModel(VesselData& vd, const VesselData& vd_prev);

  //! Calculate score of a vessel template, i.e., how well it fits the image
  virtual void _calcScore(VesselData& vd);

  //! Determines a suitable template size in voxels. The half size is returned for
  //! each dimension (x,y,z) is returned. That is, if n is the half side, 2*n+1 is
  //! the full size.
  virtual void _getPatchSize(int voxelHalfSize[3], double radius);

  //! Linear least squares fit for contrast and mean parameters.
  void _lsFit(VesselTemplate& vt, const ImPatch& T, const ImPatch& I, const ImPatch& W);

  void _calcScore(VesselTemplate& vt, const ImPatch& T, const ImPatch& I, const ImPatch& W);

  //! A Gaussian weightfunction.
  void _weightFunction(ImPatch& W, const VesselData& vd);

  double _getWeightThreshold();

  //! Returns a suitable size of the window function.
  double _getWindowSigma(double radius);

  //! Get an image patch adapted to the vessel data in VesselData.
  void _getImPatch(ImPatch& I, const VesselData& vd);



private:

    VesselTemplate _vt;

    //! Width factor of the Gaussian weight window. sigma = vesselRadius*windowSizeFactor.
    double  _windowSizeFactor;

    // ImPatches for template, weight and image data
    ImPatch _T;
    ImPatch _W;
    ImPatch _I;
    ImPatch _trial_T;

    // Vector holding residuals, i.e., remaining image noise
    // when the fitted template has been substracted from the
    // image patch.
    std::valarray<double> _wresiduals;
};

#endif
